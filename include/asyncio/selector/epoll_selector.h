//
// Created by netcan on 2021/10/09.
//

#ifndef ASYNCIO_EPOLL_SELECTOR_H
#define ASYNCIO_EPOLL_SELECTOR_H
#include "event.h"
#include <unistd.h>
#ifdef _WIN32
#include <wepoll.h>
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
typedef HANDLE epoll_handle_t;
#else
#include <sys/epoll.h>
typedef int epoll_handle_t;
#endif // _WIN32
#include <vector>

namespace asyncio
{

struct EpollSelector {
    EpollSelector(): epfd_(epoll_create1(0)) {
#ifdef _WIN32
        if (epfd_ == INVALID_HANDLE_VALUE) {
#else
        if (epfd_ < 0) {
#endif // _WIN32
            perror("epoll_create1");
            throw;
        }
    }
    void select(int timeout /* ms */, std::vector<Event>& events) {
        errno = 0;
        std::vector<epoll_event> out_events;
        out_events.resize(register_event_count_);
        int ndfs = epoll_wait(epfd_, out_events.data(), register_event_count_, timeout);
        for (size_t i = 0; i < ndfs; ++i) {
            auto handle_info = reinterpret_cast<HandleInfo*>(out_events[i].data.ptr);
            if (handle_info->handle != nullptr && handle_info->handle != (Handle*)&handle_info->handle) {
                events.emplace_back(Event {
                    .handle_info = *handle_info
                });
            } else {
                // mark event ready, but has no response callback
                handle_info->handle = (Handle*)&handle_info->handle;
            }
        }
    }
    ~EpollSelector() {
#ifdef _WIN32
        if (epfd_ != INVALID_HANDLE_VALUE) {
            epoll_close(epfd_);
        }
#else
        if (epfd_ > 0) {
            close(epfd_);
        }
#endif // _WIN32
    }
    bool is_stop() { return register_event_count_ == 1; }
    void register_event(const Event& event) {
        epoll_event ev{ .events = event.flags, .data {.ptr = const_cast<HandleInfo*>(&event.handle_info) } };
        if (epoll_ctl(epfd_, EPOLL_CTL_ADD, event.fd, &ev) == 0) {
            ++register_event_count_;
        }
    }

    void remove_event(const Event& event) {
        epoll_event ev{ .events = event.flags };
        if (epoll_ctl(epfd_, EPOLL_CTL_DEL, event.fd, &ev) == 0) {
            --register_event_count_;
        }
    }
private:
    epoll_handle_t epfd_;
    int register_event_count_ {1};
};

} // namespace asyncio

#endif // ASYNCIO_EPOLL_SELECTOR_H
