#ifndef COMMON_TUPLE_UTIL_H_
#define COMMON_TUPLE_UTIL_H_

#include <cstddef>

#include <ostream>
#include <tuple>
#include <type_traits>
#include <vector>

#include "common/macros.h"
#include "common/sizet_list.h"
#include "common/status.h"
#include "common/status_macros.h"
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

// Iteri (non-const)
template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I == sizeof...(Ts)>::type TupleIteriImpl(
    std::tuple<Ts...>&, F&) {}

template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I != sizeof...(Ts)>::type TupleIteriImpl(
    std::tuple<Ts...>& t, F& f) {
  f(I, std::get<I>(t));
  TupleIteriImpl<I + 1>(t, f);
}

// Iteri (const)
template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I == sizeof...(Ts)>::type TupleIteriImpl(
    const std::tuple<Ts...>&, F&) {}

template <std::size_t I, typename F, typename... Ts>
typename std::enable_if<I != sizeof...(Ts)>::type TupleIteriImpl(
    const std::tuple<Ts...>& t, F& f) {
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

// Fold (non-const)
template <std::size_t I, typename F, typename Acc, typename... Ts>
typename std::enable_if<I == sizeof...(Ts), Acc>::type TupleFoldImpl(
    const Acc& acc, std::tuple<Ts...>&, const F&) {
  return acc;
}

template <std::size_t I, typename F, typename Acc, typename... Ts>
typename std::enable_if<I != sizeof...(Ts), Acc>::type TupleFoldImpl(
    const Acc& acc, std::tuple<Ts...>& t, F& f) {
  return TupleFoldImpl<I + 1>(f(acc, std::get<I>(t)), t, f);
}

// Fold (const)
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
Acc TupleFold(const Acc& acc, std::tuple<Ts...>& t, F f) {
  return detail::TupleFoldImpl<0>(acc, t, f);
}

template <typename F, typename Acc, typename... Ts>
Acc TupleFold(const Acc& acc, const std::tuple<Ts...>& t, F f) {
  return detail::TupleFoldImpl<0>(acc, t, f);
}

// `TupleIteriStatus((a, ..., z), f)` executes the following;
//
//   RETURN_IF_ERROR(f(0, a));
//   RETURN_IF_ERROR(f(1, b));
//   ...
//   RETURN_IF_ERROR(f(25, z));
template <typename F, typename... Ts>
WARN_UNUSED Status TupleIteriStatus(std::tuple<Ts...>& t, F f) {
  using Acc = std::pair<Status, std::size_t>;
  return std::get<0>(
      TupleFold(Acc(Status::OK, 0), t, [&f](const Acc& acc, auto& x) {
        const auto& status = std::get<0>(acc);
        std::size_t i = std::get<1>(acc);
        return status.ok() ? Acc(f(i, x), i + 1) : acc;
      }));
}

template <typename F, typename... Ts>
WARN_UNUSED Status TupleIteriStatus(const std::tuple<Ts...>& t, F f) {
  using Acc = std::pair<Status, std::size_t>;
  return std::get<0>(
      TupleFold(Acc(Status::OK, 0), t, [&f](const Acc& acc, const auto& x) {
        const auto& status = std::get<0>(acc);
        std::size_t i = std::get<1>(acc);
        return status.ok() ? Acc(f(i, x), i + 1) : acc;
      }));
}

// `TupleIterStatus((a, ..., z), f)` executes the following;
//
//   RETURN_IF_ERROR(f(a));
//   RETURN_IF_ERROR(f(b));
//   ...
//   RETURN_IF_ERROR(f(z));
template <typename F, typename... Ts>
WARN_UNUSED Status TupleIterStatus(std::tuple<Ts...>& t, F f) {
  return TupleFold(Status::OK, t, [&f](const auto& status, auto& x) {
    return status.ok() ? f(x) : status;
  });
}

template <typename F, typename... Ts>
WARN_UNUSED Status TupleIterStatus(const std::tuple<Ts...>& t, F f) {
  return TupleFold(Status::OK, t, [&f](const auto& status, const auto& x) {
    return status.ok() ? f(x) : status;
  });
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

// TupleProject<I1, ..., In>(t) = (t[I1], ..., t[In])
template <std::size_t... Is, typename... Ts>
auto TupleProject(const std::tuple<Ts...>& t) {
  return std::tuple_cat(std::make_tuple(std::get<Is>(t))...);
}

// TupleProject<SizetList<I1, ..., In>>(t) = (t[I1], ..., t[In])
template <typename SizetList, typename... Ts>
struct TupleProjectBySizetListImpl;

template <std::size_t... Is, typename... Ts>
struct TupleProjectBySizetListImpl<SizetList<Is...>, Ts...> {
  auto operator()(const std::tuple<Ts...>& t) { return TupleProject<Is...>(t); }
};

template <typename SizetList, typename... Ts>
auto TupleProjectBySizetList(const std::tuple<Ts...>& t) {
  return TupleProjectBySizetListImpl<SizetList, Ts...>()(t);
}

// TupleTake
template <std::size_t N, typename... Ts>
auto TupleTake(const std::tuple<Ts...>& t) {
  using ind_sequence = std::make_index_sequence<sizeof...(Ts)>;
  using sizet_list = typename SizetListFromIndexSequence<ind_sequence>::type;
  using project_ids = typename SizetListTake<sizet_list, N>::type;
  return TupleProjectBySizetList<project_ids>(t);
}

// `TupleDrop<I>()`
template <std::size_t N, typename... Ts>
auto TupleDrop(const std::tuple<Ts...>& t) {
  using ind_sequence = std::make_index_sequence<sizeof...(Ts)>;
  using sizet_list = typename SizetListFromIndexSequence<ind_sequence>::type;
  using project_ids = typename SizetListDrop<sizet_list, N>::type;
  return TupleProjectBySizetList<project_ids>(t);
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
