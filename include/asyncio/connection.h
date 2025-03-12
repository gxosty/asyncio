//
// Created by netcan on 2021/11/29.
//

#ifndef ASYNCIO_OPEN_CONNECTION_H
#define ASYNCIO_OPEN_CONNECTION_H
#include <asyncio/stream.h>
// #include <asyncio/datagram.h>
#include <asyncio/finally.h>
#include <asyncio/selector/event.h>
#include <exception>
#include <asyncio/task.h>
#include <fcntl.h>
#include <sys/types.h>
#include <system_error>

#ifdef ASYNCIO_WITH_SSL
    #include <asyncio/ssl_stream.h>
    #include <wolfssl/options.h>
    #include <wolfssl/ssl.h>
#endif

namespace asyncio {
namespace detail {
Task<bool> connect(int fd, const sockaddr *addr, socklen_t len) noexcept;
}

Task<Stream> open_tcp(std::string_view ip, uint16_t port);
Task<SslStream> open_ssl_tcp(WOLFSSL_CTX* ctx, std::string_view ip, uint16_t port);
// Task<Datagram> open_udp(std::string_view ip, uint16_t port);

} // namespace asyncio

#endif // ASYNCIO_OPEN_CONNECTION_H
