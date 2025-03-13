#ifndef ASYNCIO_SSLSTREAM_H
#define ASYNCIO_SSLSTREAM_H

#include "event_loop.h"
#include "selector/event.h"
#include "noncopyable.h"
#include "task.h"
#include "socket.h"
#include "types.h"
#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include <vector>

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

namespace asyncio
{

struct SslStream: NonCopyable {
    SslStream(WOLFSSL* ssl, int fd)
        : ssl_(ssl)
        , read_sock_(fd)
        , write_sock_(Socket::duplicate(read_sock_))
        , read_ev_(read_sock_.get_fd(), Event::Flags::EVENT_READ)
        , write_ev_(write_sock_.get_fd(), Event::Flags::EVENT_WRITE)
        , read_awaiter_(get_event_loop().wait_event(read_ev_))
        , write_awaiter_(get_event_loop().wait_event(write_ev_)) { }

    SslStream(SslStream&& other)
        : ssl_(std::exchange(other.ssl_, nullptr))
        , read_sock_(other.read_sock_.detach())
        , write_sock_(other.write_sock_.detach())
        , read_ev_(std::move(other.read_ev_))
        , write_ev_(std::move(other.write_ev_))
        , read_awaiter_(std::move(other.read_awaiter_))
        , write_awaiter_(std::move(other.write_awaiter_)) {}

    ~SslStream() { close(); }

    void close() {
        if (ssl_) {
            wolfSSL_shutdown(ssl_);
            wolfSSL_free(ssl_);
            ssl_ = nullptr;
        }
        read_awaiter_.destroy();
        write_awaiter_.destroy();
        read_sock_.close();
        write_sock_.close();
    }

    Task<Buffer> read(ssize_t sz = -1) {
        if (sz < 0) { co_return co_await read_until_eof(); }

        Buffer result(sz, 0);
        co_await read_awaiter_;
        sz = wolfSSL_read(ssl_, result.data(), result.size());
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
            // FIXME: how to handle write event?
            // co_await write_awaiter_;
            ssize_t sz = wolfSSL_write(ssl_, buf.data() + total_write, buf.size() - total_write);
            if (sz == -1) {
                throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
            }
            total_write += sz;
        }
        co_return;
    }

private:
    Task<Buffer> read_until_eof() {
        auto& loop = get_event_loop();

        Buffer result(chunk_size, 0);
        int current_read = 0;
        int total_read = 0;
        do {
            co_await read_awaiter_;
            current_read = wolfSSL_read(ssl_, result.data() + total_read, chunk_size);
            if (current_read == -1) {
                throw std::system_error(std::make_error_code(static_cast<std::errc>(errno)));
            }
            if (current_read < chunk_size) { result.resize(total_read + current_read); }
            total_read += current_read;
            result.resize(total_read + chunk_size);
        } while (current_read > 0);
        co_return result;
    }
private:
    WOLFSSL* ssl_{nullptr};
    Socket read_sock_;
    Socket write_sock_;
    Event read_ev_;
    Event write_ev_;
    EventLoop::WaitEventAwaiter read_awaiter_;
    EventLoop::WaitEventAwaiter write_awaiter_;
    constexpr static size_t chunk_size = 4096;
};

} // namespace asyncio

#endif // ASYNCIO_SSLSTREAM_H
