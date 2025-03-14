//
// Created by netcan on 2021/11/21.
//

#ifndef ASYNCIO_EXCEPTION_H
#define ASYNCIO_EXCEPTION_H
#include <stdexcept>
#include <string>

namespace asyncio
{

struct TimeoutError: std::exception {
    [[nodiscard]] const char* what() const noexcept override {
        return "TimeoutError";
    }
};

struct NoResultError: std::exception {
    [[nodiscard]] const char* what() const noexcept override {
        return "result is unset";
    }
};

struct InvalidFuture: std::exception {
    [[nodiscard]] const char* what() const noexcept override {
        return "future is invalid";
    }
};

struct SocketCreationError : std::exception {
    [[nodiscard]] const char* what() const noexcept override {
        return "socket creation failed (socket == -1)";
    }
};

struct DuplicationError : std::exception {
    [[nodiscard]] const char* what() const noexcept override {
        return "Descriptor duplication error";
    }
};

struct SslHandshakeError : std::exception {
    SslHandshakeError(int errcode) : msg("SSL Handshake error: " + std::to_string(errcode)) {}

    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }

    std::string msg;
};

struct SocketIOError : std::exception {
    SocketIOError(int errcode) : msg("Socket IO error: " + std::to_string(errcode)) {}

    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }

    std::string msg;
};

}

#endif // ASYNCIO_EXCEPTION_H
