#ifndef COMMON_TUPLE_UTIL_H_
#define COMMON_TUPLE_UTIL_H_

#include <cstddef>

#include <ostream>
#include <tuple>
#include <type_traits>
#include <vector>

#include "common/type_list.h"

// From functional programming languages like Hasell and OCaml, we're used to
// seeing a generic set of functions that operate on lists:
//
//   - map: ('a -> 'b) -> 'a list -> 'b list
//   - iter: ('a -> unit) -> 'a list -> unit
//   - fold_left: ('a -> 'b -> 'a) -> 'a -> 'b list -> 'a
//
// These functions exist in C++ for iterables too (see <algorithm>). However,
// once we stop using iterators and start using tuples, we lose all these nice
// functions. Want to call `iter` on a tuple to print out its elements? Too
// bad. Want to map a function over the elements of a tuple to construct a
// different tuple? Sorry, out of luck.
//
// This file implements the set of list functions we're used to but for tuples:
//
//   // Print every element of a tuples.
//   tuple<int, char, bool> t{1, 'a', true};
//   TupleIter(t, [](auto x) { cout << x << endl; });
//
//   // Map a function over a tuple.
//   tuple<string, string, string> s = TupleMap(t, [](auto x) {
//     return to_string(x);
//   });
//
//   // Sum the values in a tuple.
//   tuple<int, float, double> nums{1, 2.0, 3.0};
//   double sum = TupleFold(0.0, nums, [](auto acc, auto x) {
//     return acc + x;
//   });
//
//   // Convert a tuple to a vector.
//   tuple<int, int, int> int_tuple{1, 2, 3};
//   std::vector<int> int_vector = TupletoVector(int_tuple);

namespace fluent {
namespace detail {

// Iteri
template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I == sizeof...(Ts)>::type TupleIteriImpl(
    const std::tuple<Ts...>&, F&) {}

template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I != sizeof...(Ts)>::type TupleIteriImpl(
    const std::tuple<Ts...>& t, F& f) {
  f(I, std::get<I>(t));
  TupleIteriImpl<I + 1>(t, f);
}

template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I == sizeof...(Ts)>::type TupleIteriImpl(
    std::tuple<Ts...>&, F&) {}

template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I != sizeof...(Ts)>::type TupleIteriImpl(
    std::tuple<Ts...>& t, F& f) {
  f(I, std::get<I>(t));
  TupleIteriImpl<I + 1>(t, f);
}

// Map
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

// Fold
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

}  // namespace detail

// `TupleIteri((x1, ..., xn), f)` executes `f(1, x1); ...; f(n, xn)`.
template <typename F, typename... Ts>
void TupleIteri(const std::tuple<Ts...>& t, F f) {
  detail::TupleIteriImpl<0>(t, f);
}

template <typename F, typename... Ts>
void TupleIteri(std::tuple<Ts...>& t, F f) {
  detail::TupleIteriImpl<0>(t, f);
}

// `TupleIter((a, ..., z), f)` executes `f(a); ...; f(z)`.
template <typename F, typename... Ts>
void TupleIter(const std::tuple<Ts...>& t, F f) {
  TupleIteri(t, [&f](std::size_t, const auto& x) { f(x); });
}

template <typename F, typename... Ts>
void TupleIter(std::tuple<Ts...>& t, F f) {
  TupleIteri(t, [&f](std::size_t, auto& x) { f(x); });
}

// `TupleMap((a, ..., z), f)` returns the tuple `(f(a), ..., f(z))`.
template <typename F, typename... Ts>
std::tuple<typename std::result_of<F(const Ts&)>::type...> TupleMap(
    const std::tuple<Ts...>& t, F f) {
  return detail::TupleMapImpl<0>(t, f);
}

// `TupleFold(a, (x, y, z), f)` returns `f(f(f(a, x), y), z)`.
template <typename F, typename Acc, typename... Ts>
Acc TupleFold(const Acc& acc, const std::tuple<Ts...>& t, F f) {
  return detail::TupleFoldImpl<0>(acc, t, f);
}

// `TupleToVector(t)` converts `t` into a vector.
template <typename T, typename... Ts>
std::vector<T> TupleToVector(const std::tuple<T, Ts...>& t) {
  static_assert(TypeListAllSame<TypeList<T, Ts...>>::value,
                "TupletoVector expects a tuple with a single type.");
  std::vector<T> xs(sizeof...(Ts) + 1);
  TupleIteri(t, [&xs](std::size_t i, const auto& x) { xs[i] = x; });
  return xs;
}

// `TupleFromTypes<F, T1,...,Tn>` returns the tuple `(F<T1>()(), ...,
// F<Tn>()())`.
template <template <typename> class F, typename... Ts>
auto TupleFromTypes() {
  return std::make_tuple(F<Ts>()()...);
}

// TupleProject<I1, ..., In>(t) = (t[I1], ..., t[In])
template <std::size_t... Is, typename... Ts>
auto TupleProject(const std::tuple<Ts...>& t) {
  return std::tuple_cat(std::make_tuple(std::get<Is>(t))...);
}

// << operator
template <typename... Ts>
std::ostream& operator<<(std::ostream& out, const std::tuple<Ts...>& t) {
  out << "(";
  TupleIteri(t, [&out](std::size_t i, const auto& t) {
    if (i == sizeof...(Ts) - 1) {
      out << t;
    } else {
      out << t << ", ";
    }
  });
  out << ")";
  return out;
}

}  // namespace fluent

#endif  //  COMMON_TUPLE_UTIL_H_