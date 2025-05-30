#include <asyncio/connection.h>
#include <asyncio/event_loop.h>
#include <asyncio/exception.h>
#include <asyncio/errno.h>

#ifdef _WIN32
    #include <ws2tcpip.h>    
#else
    #include <arpa/inet.h>
#endif

namespace asyncio
{

Task<bool> _connect(int fd, const sockaddr *addr, socklen_t len) noexcept {
    int rc = ::connect(fd, addr, len);
    if (rc == 0) { co_return true; }
    if (rc < 0
#ifdef _WIN32
            && sockerrno != WSAEWOULDBLOCK
#else
            && sockerrno != EINPROGRESS
#endif // _WIN32
    ) {
        throw SocketIOError(sockerrno);
    }
    Event ev { .fd = fd, .flags = Event::Flags::EVENT_WRITE };
    auto& loop = get_event_loop();
    co_await loop.wait_event(ev);

    int result{0};
    socklen_t result_len = sizeof(result);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&result, &result_len) < 0) {
        // error, fail somehow, close socket
        co_return false;
    }
    co_return result == 0;
}

Task<int> open_socket_raw(std::string_view ip, uint16_t port, int type, int protocol)
{
    sockaddr_storage addr;
    addr.ss_family = AF_INET;
    if (inet_pton(AF_INET, (const char*)ip.data(), &((sockaddr_in*)&addr)->sin_addr))
    {
        ((sockaddr_in*)&addr)->sin_port = htons(port);
    }
    else
    {
        addr.ss_family = AF_INET6;
        inet_pton(AF_INET6, (const char*)ip.data(), &((sockaddr_in6*)&addr)->sin6_addr);
        ((sockaddr_in6*)&addr)->sin6_port = htons(port);
    }
    auto sock = Socket(addr.ss_family, type, protocol);
    sock.set_blocking(false);
    int sockfd = sock.detach();

    if (sockfd == -1)
    {
        throw SocketCreationError();
    }

    if (!co_await _connect(
            sockfd,
            (sockaddr*)&addr,
            addr.ss_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6)))
    {
        close(sockfd);
        throw SocketIOError(sockerrno);
    }
    
    co_return sockfd;
}

Task<Stream> open_tcp(std::string_view ip, uint16_t port) {
    int sockfd = co_await open_socket_raw(ip, port, SOCK_STREAM, IPPROTO_TCP);
    co_return Stream(sockfd);
}

Task<Datagram> open_udp(std::string_view ip, uint16_t port) {
    int sockfd = co_await open_socket_raw(ip, port, SOCK_DGRAM, IPPROTO_UDP);
    co_return Datagram(sockfd);
}

#ifdef ASYNCIO_WITH_SSL

Task<int> _ssl_connect(WOLFSSL* ssl, int sockfd)
{
    auto& loop = get_event_loop();
    while ((wolfSSL_connect(ssl)) != WOLFSSL_SUCCESS) {
        int err = wolfSSL_get_error(ssl, 0);
        if (err == WOLFSSL_ERROR_WANT_READ) {
            Event ev { .fd = sockfd, .flags = Event::Flags::EVENT_READ };
            co_await loop.wait_event(ev);
        } else if (err == WOLFSSL_ERROR_WANT_WRITE) {
            Event ev { .fd = sockfd, .flags = Event::Flags::EVENT_WRITE };
            co_await loop.wait_event(ev);
        } else {
            co_return err;
        }
    }

    co_return WOLFSSL_SUCCESS;
}

Task<SslStream> open_ssl_tcp(WOLFSSL_CTX* ctx, std::string_view ip, uint16_t port) {
    int sockfd = co_await open_socket_raw(ip, port, SOCK_STREAM, IPPROTO_TCP);

    WOLFSSL* ssl = wolfSSL_new(ctx);
    wolfSSL_set_fd(ssl, sockfd);

    int res = co_await _ssl_connect(ssl, sockfd);

    if (res != WOLFSSL_SUCCESS)
    {
        wolfSSL_free(ssl);
        ::close(sockfd);
        throw SslHandshakeError(res);
    }

    co_return SslStream(ssl, sockfd);
}

#endif // ASYNCIO_WITH_SSL

} // namespace asyncio
