//
// Created by netcan on 2021/10/24.
//

#ifndef ASYNCIO_EVENT_H
#define ASYNCIO_EVENT_H

#include <asyncio/asyncio_ns.h>
#include <asyncio/handle.h>
#include <cstdint>

#if defined(__APPLE__)
    #include <sys/event.h>
    using Flags_t = int16_t;
#elif defined(__linux__)
    #include <sys/epoll.h>
    using Flags_t = uint32_t;
#elif defined (_WIN32)
    #include <winsock2.h>
    using Flags_t = uint32_t;
#endif

ASYNCIO_NS_BEGIN
struct Event {
    enum Flags: Flags_t {
    #if defined(__APPLE__)
        EVENT_READ = EVFILT_READ,
        EVENT_WRITE = EVFILT_WRITE
    #elif defined(__linux__)
        EVENT_READ = EPOLLIN,
        EVENT_WRITE = EPOLLOUT
    #elif defined(_WIN32)
        EVENT_READ = POLLIN,
        EVENT_WRITE = POLLOUT
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
ASYNCIO_NS_END

#endif //ASYNCIO_EVENT_H
