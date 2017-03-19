#ifndef COMMON_TUPLE_UTIL_H_
#define COMMON_TUPLE_UTIL_H_

#include <cstddef>

#include <tuple>
#include <type_traits>

namespace fluent {
namespace {

template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I == sizeof...(Ts)>::type TupleIterImpl(
    const std::tuple<Ts...>&, const F&) {}

template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I != sizeof...(Ts)>::type TupleIterImpl(
    const std::tuple<Ts...>& t, F& f) {
  f(std::get<I>(t));
  TupleIterImpl<I + 1>(t, f);
}

}  // namespace

// `TupleIter(f, (a, ..., z))` executes `f(a); ...; f(z)`.
template <typename F, typename... Ts>
void TupleIter(const std::tuple<Ts...>& t, F f) {
  TupleIterImpl<0>(t, f);
}

}  // namespace fluent

#endif  //  COMMON_TUPLE_UTIL_H_
