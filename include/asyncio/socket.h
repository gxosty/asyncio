#pragma once

#include <cstdint>

namespace asyncio
{

/**
 * Wrapper class around raw socket
 */
class Socket
{
public:
    Socket() : _fd{-1} {}
    Socket(int domain, int type, int protocol);
    Socket(int fd) : _fd{fd} {}
    Socket(Socket&) = delete;
    Socket(Socket&&);
    ~Socket();

    static Socket duplicate(const Socket& other);

    inline int get_fd() const noexcept
    {
        return _fd;
    }

    void close();
    int detach();
    
    bool set_blocking(bool enabled);

    size_t recv(char* buffer, size_t buffer_size, int flags = 0);
    size_t send(const char* buffer, size_t buffer_size, int flags = 0);

private:
    int _fd;
};

}
