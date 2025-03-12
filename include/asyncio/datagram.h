#ifndef ASYNCIO_DATAGRAM_H
#define ASYNCIO_DATAGRAM_H

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

#include <asyncio/asyncio_ns.h>
#include <asyncio/event_loop.h>
#include <asyncio/selector/event.h>
#include <asyncio/noncopyable.h>
#include <asyncio/task.h>
#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include <vector>

ASYNCIO_NS_BEGIN
struct Datagram: NonCopyable {
    using Buffer = std::vector<char>;
    Datagram(int fd): read_fd_(fd), write_fd_(dup(fd)) {
        if (read_fd_ >= 0) {
            socklen_t addrlen = sizeof(sock_info_);
            getsockname(read_fd_, reinterpret_cast<sockaddr*>(&sock_info_), &addrlen);
        }
    }
    Datagram(int fd, const sockaddr_storage& sockinfo): read_fd_(fd), write_fd_(dup(fd)), sock_info_(sockinfo) { }
    Datagram(Datagram&& other): read_fd_{std::exchange(other.read_fd_, -1) },
                            write_fd_{std::exchange(other.write_fd_, -1) },
                            read_ev_{ std::exchange(other.read_ev_, {}) },
                            write_ev_{ std::exchange(other.write_ev_, {}) },
                            read_awaiter_{ std::move(other.read_awaiter_) },
                            write_awaiter_{ std::move(other.write_awaiter_) },
                            sock_info_{ other.sock_info_ } { }
    ~Datagram() { close(); }

    void close() {
        read_awaiter_.destroy();
        write_awaiter_.destroy();
        if (read_fd_ > 0) { ::close(read_fd_); }
        if (write_fd_ > 0) { ::close(write_fd_); }
        read_fd_ = -1;
        write_fd_ = -1;
    }

    Task<Buffer> read(size_t sz) {
        Buffer result(sz, 0);
        co_await read_awaiter_;
        sz = ::read(read_fd_, result.data(), result.size());
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
            ssize_t sz = ::write(write_fd_, buf.data() + total_write, buf.size() - total_write);
            if (sz == -1) {
                throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
            }
            total_write += sz;
        }
        co_return;
    }
    const sockaddr_storage& get_sock_info() const {
        return sock_info_;
    }

private:
    int read_fd_{-1};
    int write_fd_{-1};
    Event read_ev_ { .fd = read_fd_, .flags = Event::Flags::EVENT_READ };
    Event write_ev_ { .fd = write_fd_, .flags = Event::Flags::EVENT_WRITE };
    EventLoop::WaitEventAwaiter read_awaiter_ { get_event_loop().wait_event(read_ev_) };
    EventLoop::WaitEventAwaiter write_awaiter_ { get_event_loop().wait_event(write_ev_) };
    sockaddr_storage sock_info_{};
    constexpr static size_t chunk_size = 4096;
};

ASYNCIO_NS_END

#endif // ASYNCIO_DATAGRAM_H
