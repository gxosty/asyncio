//
// Created by netcan on 2021/11/29.
//

#ifndef ASYNCIO_OPEN_CONNECTION_H
#define ASYNCIO_OPEN_CONNECTION_H
#include "stream.h"
#include "datagram.h"
#include "task.h"

#ifdef ASYNCIO_WITH_SSL
    #include "ssl_stream.h"
    #include <wolfssl/options.h>
    #include <wolfssl/ssl.h>
#endif

namespace asyncio {

Task<Stream> open_tcp(std::string_view ip, uint16_t port);
Task<Datagram> open_udp(std::string_view ip, uint16_t port);

#ifdef ASYNCIO_WITH_SSL
Task<SslStream> open_ssl_tcp(WOLFSSL_CTX* ctx, std::string_view ip, uint16_t port);
#endif // ASYNCIO_WITH_SSL

} // namespace asyncio

#endif // ASYNCIO_OPEN_CONNECTION_H
