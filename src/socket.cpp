#include <asyncio/socket.h>
#include <asyncio/exception.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
#endif

#include <fcntl.h>

namespace asyncio
{

Socket Socket::duplicate(const Socket& other)
{
#ifdef _WIN32
    WSAPROTOCOL_INFO protocol_info;

    if (WSADuplicateSocket(other._fd, GetCurrentProcessId(), &protocol_info) == SOCKET_ERROR)
    {
        throw DuplicationError();
    }

    int dup_fd = WSASocket(protocol_info.iAddressFamily, protocol_info.iSocketType,
            protocol_info.iProtocol, &protocol_info, 0, 0);
    // int dup_fd = other._fd;
#else
    int dup_fd = dup(other._fd);
#endif

    return Socket(dup_fd);
}

Socket::Socket(int domain, int type, int protocol)
{
#ifdef _WIN32
    _fd = WSASocket(domain, type, protocol, NULL, 0, 0);
#else
    _fd = socket(domain, type, protocol);
#endif
}

Socket::Socket(Socket&& other)
{
    _fd = other._fd;
    other._fd = -1;
}

Socket::~Socket()
{
    this->close();
}

void Socket::close()
{
    if (_fd != -1)
    {
#ifdef _WIN32
        closesocket(_fd);
#else
        close(_fd);
#endif
        _fd = -1;
    }
}


int Socket::detach()
{
    int fd = _fd;
    _fd = -1;
    return fd;
}

bool Socket::set_blocking(bool enabled)
{
#if defined(_WIN32)
    unsigned long block = !enabled;
    return !ioctlsocket(_fd, FIONBIO, &block);
#elif __has_include(<sys/ioctl.h>) && defined(FIONBIO)
    unsigned int block = !enabled;
    return !ioctl(_fd, FIONBIO, &block);
#else
    int delay_flag, new_delay_flag;
    delay_flag = fcntl(_fd, F_GETFL, 0);
    if (delay_flag == -1)
        return false;
    new_delay_flag = enabled ? (delay_flag & ~O_NONBLOCK) : (delay_flag | O_NONBLOCK);
    if (new_delay_flag != delay_flag)
        return !fcntl(_fd, F_SETFL, new_delay_flag);
    return false;
#endif
}

size_t Socket::recv(char* buffer, size_t buffer_size, int flags)
{
    return ::recv(_fd, buffer, buffer_size, flags);
}

size_t Socket::send(const char* buffer, size_t buffer_size, int flags)
{
    return ::send(_fd, buffer, buffer_size, flags);
}

}
