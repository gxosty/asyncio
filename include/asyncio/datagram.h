#ifndef ASYNCIO_DATAGRAM_H
#define ASYNCIO_DATAGRAM_H

#include "event_loop.h"
#include "selector/event.h"
#include "noncopyable.h"
#include "task.h"
#include "socket.h"
#include "types.h"
#include "errno.h"
#include <unistd.h>
#include <utility>
#include <vector>

namespace asyncio
{

struct Datagram: NonCopyable {
    Datagram(int fd)
        : read_sock_(fd)
        , write_sock_(Socket::duplicate(read_sock_))
        , read_ev_(read_sock_.get_fd(), Event::Flags::EVENT_READ)
        , write_ev_(write_sock_.get_fd(), Event::Flags::EVENT_WRITE)
        , read_awaiter_(get_event_loop().wait_event(read_ev_))
        , write_awaiter_(get_event_loop().wait_event(write_ev_)) { }

    Datagram(Datagram&& other)
        : read_sock_(other.read_sock_.detach())
        , write_sock_(other.write_sock_.detach())
        , read_ev_(std::move(other.read_ev_))
        , write_ev_(std::move(other.write_ev_))
        , read_awaiter_(std::move(other.read_awaiter_))
        , write_awaiter_(std::move(other.write_awaiter_)) {}

    ~Datagram() { close(); }

    void close() {
        read_awaiter_.destroy();
        write_awaiter_.destroy();
        read_sock_.close();
        write_sock_.close();
    }

    Task<Buffer> read(size_t sz) {
        Buffer result(sz, 0);
        co_await read_awaiter_;
        sz = read_sock_.recv((char*)result.data(), sz, 0);
        if (sz == -1) {
            throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
        }
        result.resize(sz);
        co_return result;
    }

    Task<> write(const Buffer& buf) {
        auto& loop = get_event_loop();
        ssize_t total_write = 0;
        while (total_write < buf.size()) {
            ssize_t sz = write_sock_.send(buf.data() + total_write, buf.size() - total_write, 0);
            if (sz == -1) {
                throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
            }
            total_write += sz;
        }
        co_return;
    }

private:
    Socket read_sock_;
    Socket write_sock_;
    Event read_ev_;
    Event write_ev_;
    EventLoop::WaitEventAwaiter read_awaiter_;
    EventLoop::WaitEventAwaiter write_awaiter_;
    constexpr static size_t chunk_size = 4096;
};

} // namespace asyncio

#endif // ASYNCIO_DATAGRAM_H

