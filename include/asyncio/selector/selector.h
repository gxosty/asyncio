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
#elif defined(__linux)
#include "epoll_selector.h"
namespace asyncio {
using Selector = EpollSelector;
}
#elif defined(_WIN32)
#include "wsapoll_selector.h"
namespace asyncio {
using Selector = WSAPollSelector;
}
#endif

#endif //ASYNCIO_SELECTOR_H
