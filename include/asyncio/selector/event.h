//
// Created by netcan on 2021/10/24.
//

#ifndef ASYNCIO_EVENT_H
#define ASYNCIO_EVENT_H

#include "../handle.h"

#include <cstdint>
#if defined(__APPLE__)
    #include <sys/event.h>
    using Flags_t = int16_t;
#elif defined(__linux__)
    #include <sys/epoll.h>
    using Flags_t = uint32_t;
#elif defined(_WIN32)
    #include "win32_selector_defs.h"
    #if defined(_ASYNCIO_WIN32_SELECTOR_WSAPOLL)
        #include <winsock2.h>
    #elif defined(_ASYNCIO_WIN32_SELECTOR_EPOLL)
        #include <wepoll.h>
    #endif // _ASYNCIO_WIN32_SELECTOR

    using Flags_t = uint32_t;
#endif

namespace asyncio
{

struct Event {
    enum Flags: Flags_t {
    #if defined(__APPLE__)
        EVENT_READ = EVFILT_READ,
        EVENT_WRITE = EVFILT_WRITE
    #elif defined(__linux__)
        EVENT_READ = EPOLLIN,
        EVENT_WRITE = EPOLLOUT
    #elif defined(_WIN32)
        #if defined(_ASYNCIO_WIN32_SELECTOR_EPOLL)
        EVENT_READ = EPOLLIN,
        EVENT_WRITE = EPOLLOUT
        #elif defined(_ASYNCIO_WIN32_SELECTOR_WSAPOLL)
        EVENT_READ = POLLIN,
        EVENT_WRITE = POLLOUT
        #endif
    #else
        #error "Unsupported platform"
    #endif
    };

    int fd;
    Flags flags;
    HandleInfo handle_info;

    inline bool operator==(const Event& other) const
    {
        return ((fd == other.fd) && (flags == other.flags));
    }
};

} // namespace asyncio

#endif //ASYNCIO_EVENT_H
