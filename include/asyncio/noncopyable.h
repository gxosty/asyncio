//
// Created by netcan on 2021/09/08.
//

#ifndef ASYNCIO_NONCOPYABLE_H
#define ASYNCIO_NONCOPYABLE_H

namespace asyncio
{

struct NonCopyable {
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

} // namespace asyncio

#endif // ASYNCIO_NONCOPYABLE_H
