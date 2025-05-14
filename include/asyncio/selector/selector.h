//
// Created by netcan on 2021/10/24.
//

#ifndef ASYNCIO_SELECTOR_H
#define ASYNCIO_SELECTOR_H

#if defined(__APPLE__)
#include "kqueue_selector.h"
namespace asyncio {
using Selector = KQueueSelector;
}
#elif defined(__linux__)
#include "epoll_selector.h"
namespace asyncio {
using Selector = EpollSelector;
}
#elif defined(_WIN32)

#if !defined(ASYNCIO_WIN32_SELECTOR_EPOLL) && !defined(ASYNCIO_WIN32_SELECTOR_WSAPOLL)
#define _ASYNCIO_WIN32_SELECTOR_EPOLL
#else
#if defined(ASYNCIO_WIN32_SELECTOR_EPOLL)
#define _ASYNCIO_WIN32_SELECTOR_EPOLL
#else
#define _ASYNCIO_WIN32_SELECTOR_WSAPOLL
#endif // ASYNCIO_WIN32_SELECTOR_EPOLL
#endif // !ASYNCIO_WIN32_SELECTOR_EPOLL && !ASYNCIO_WIN32_SELECTOR_WSAPOLL

#if defined(_ASYNCIO_WIN32_SELECTOR_EPOLL)
    #include "epoll_selector.h"
    namespace asyncio {
    using Selector = EpollSelector;
    }
#elif defined(_ASYNCIO_WIN32_SELECTOR_WSAPOLL)
    #include "wsapoll_selector.h"
    namespace asyncio {
    using Selector = WSAPollSelector;
    }
#endif // _ASYNCIO_WIN32_SELECTOR
#endif // _WIN32

#endif // ASYNCIO_SELECTOR_H
