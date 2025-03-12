#pragma once

#include "event.h"

#include <vector>

namespace asyncio
{

class WSAPollSelector
{
public:
    WSAPollSelector() = default;

    void select(int timeout /* ms */, std::vector<Event>& events)
    {
        if (_events.empty())
        {
            return;
        }

        std::vector<WSAPOLLFD> fds(_events.size());

        for (int i = 0; i < _events.size(); i++)
        {
            fds[i].fd = _events[i].fd;
            fds[i].events = _events[i].flags;
            fds[i].revents = 0;
        }

        int res = WSAPoll((LPWSAPOLLFD)fds.data(), fds.size(), timeout);
        
        if (res <= 0)
        {
            return;
        }

        for (int i = 0; i < fds.size(); i++)
        {
            if (fds[i].revents & fds[i].events)
            {
                auto handle_info = reinterpret_cast<HandleInfo*>(&_events[i].handle_info);
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
    }

    bool is_stop()
    {
        return _events.empty();
    }

    void register_event(const Event& event)
    {
        _events.push_back(event);
    }

    void remove_event(const Event& event)
    {
        for (auto it = _events.begin(); it < _events.end(); it++)
        {
            if ((*it) == event)
            {
                _events.erase(it);
                break;
            }
        }
    }

private:
    std::vector<Event> _events;
};

} // asyncio
