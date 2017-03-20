#ifndef COMMON_TUPLE_UTIL_H_
#define COMMON_TUPLE_UTIL_H_

#include <cstddef>

#include <tuple>
#include <type_traits>

namespace fluent {
namespace {

template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I == sizeof...(Ts)>::type TupleIteriImpl(
    const std::tuple<Ts...>&, const F&) {}

template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I != sizeof...(Ts)>::type TupleIteriImpl(
    const std::tuple<Ts...>& t, F& f) {
  f(I, std::get<I>(t));
  TupleIteriImpl<I + 1>(t, f);
}

template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I == sizeof...(Ts), std::tuple<>>::type TupleMapImpl(
    const std::tuple<Ts...>&, const F&) {
  return {};
}

template <std::size_t I, typename F, typename... Ts>
auto TupleMapImpl(
    const std::tuple<Ts...>& t, F& f,
    typename std::enable_if<I != sizeof...(Ts)>::type* = nullptr) {
  return std::tuple_cat(std::make_tuple(f(std::get<I>(t))),
                        TupleMapImpl<I + 1>(t, f));
}

template <std::size_t I, typename F, typename Acc, typename... Ts>
typename std::enable_if<I == sizeof...(Ts), Acc>::type TupleFoldImpl(
    const Acc& acc, const std::tuple<Ts...>&, const F&) {
  return acc;
}

template <std::size_t I, typename F, typename Acc, typename... Ts>
typename std::enable_if<I != sizeof...(Ts), Acc>::type TupleFoldImpl(
    const Acc& acc, const std::tuple<Ts...>& t, F& f) {
  return TupleFoldImpl<I + 1>(f(acc, std::get<I>(t)), t, f);
}

}  // namespace

// `TupleIteri((x1, ..., xn), f)` executes `f(1, x1); ...; f(n, xn)`.
template <typename F, typename... Ts>
void TupleIteri(const std::tuple<Ts...>& t, F f) {
  TupleIteriImpl<0>(t, f);
}

// `TupleIter((a, ..., z), f)` executes `f(a); ...; f(z)`.
template <typename F, typename... Ts>
void TupleIter(const std::tuple<Ts...>& t, F f) {
  TupleIteri(t, [&f](std::size_t, auto& t) { f(t); });
}

// `TupleMap((a, ..., z), f)` returns the tuple `(f(a), ..., f(z))`.
template <typename F, typename... Ts>
std::tuple<typename std::result_of<F(const Ts&)>::type...> TupleMap(
    const std::tuple<Ts...>& t, F f) {
  return TupleMapImpl<0>(t, f);
}

// `TupleFold(a, (x, y, z), f)` returns `f(f(f(a, x), y), z)`.
template <typename F, typename Acc, typename... Ts>
Acc TupleFold(const Acc& acc, const std::tuple<Ts...>& t, F f) {
  return TupleFoldImpl<0>(acc, t, f);
}

}  // namespace fluent

#endif  //  COMMON_TUPLE_UTIL_H_
