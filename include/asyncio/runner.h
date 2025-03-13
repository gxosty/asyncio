//
// Created by netcan on 2021/09/07.
//

#ifndef ASYNCIO_RUNNER_H
#define ASYNCIO_RUNNER_H
#include "concept/future.h"
#include "event_loop.h"
#include "schedule_task.h"

namespace asyncio
{

template<concepts::Future Fut>
decltype(auto) run(Fut&& main) {
    auto t = schedule_task(std::forward<Fut>(main));
    get_event_loop().run_until_complete();
    if constexpr (std::is_lvalue_reference_v<Fut&&>) {
        return t.get_result();
    } else {
        return std::move(t).get_result();
    }
}

} // namespace asyncio

#endif // ASYNCIO_RUNNER_H
