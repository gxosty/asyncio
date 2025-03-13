//
// Created by netcan on 2021/11/20.
//

#ifndef ASYNCIO_VOID_VALUE_H
#define ASYNCIO_VOID_VALUE_H

namespace asyncio
{

struct VoidValue { };

namespace detail {
template<typename T>
struct GetTypeIfVoid: std::type_identity<T> {};
template<>
struct GetTypeIfVoid<void>: std::type_identity<VoidValue> {};
}

template<typename T>
using GetTypeIfVoid_t = typename detail::GetTypeIfVoid<T>::type;

} // namespace asyncio

#endif // ASYNCIO_VOID_VALUE_H
