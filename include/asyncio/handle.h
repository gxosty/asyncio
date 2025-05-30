//
// Created by netcan on 2021/09/08.
//

#ifndef ASYNCIO_HANDLE_H
#define ASYNCIO_HANDLE_H
#include <cstdint>
#include <string>
#include <source_location>

namespace asyncio
{

// for cancelled
using HandleId = uint64_t;

struct Handle { // type erase for EventLoop
    enum State: uint8_t {
        UNSCHEDULED,
        SUSPEND,
        SCHEDULED,
    };

    Handle() noexcept: handle_id_(handle_id_generation_++) {}
    virtual void run() = 0;
    void set_state(State state) { state_ = state; }
    HandleId get_handle_id() { return handle_id_; }
    virtual ~Handle() = default;
private:
    HandleId handle_id_;
    static HandleId handle_id_generation_;
protected:
    State state_ {Handle::UNSCHEDULED};
};

// handle maybe destroyed, using the increasing id to track the lifetime of handle.
// don't directly using a raw pointer to track coroutine lifetime,
// because a destroyed coroutine may has the same address as a new ready coroutine has created.
struct HandleInfo {
    HandleId id { };
    Handle* handle { };
};

struct CoroHandle: Handle {
    void schedule();
    void cancel();
private:
    virtual const std::source_location& get_frame_info() const;
};

} // namespace asyncio

#endif // ASYNCIO_HANDLE_H
