//
// Created by netcan on 2021/12/13.
//
#ifndef ASYNCIO_PROMISE_H
#include "awaitable.h"
#include "future.h"

namespace asyncio
{

namespace concepts {
template<typename P>
concept Promise = requires (P p) {
    { p.get_return_object() } -> Future;
    { p.initial_suspend() } -> Awaitable;
    { p.final_suspend() } noexcept -> Awaitable;
    p.unhandled_exception();
    requires (requires(int v) { p.return_value(v); } ||
              requires        { p.return_void();   });
};
}

} // namespace asyncio

#endif
