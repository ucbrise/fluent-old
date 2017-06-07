#ifndef COMMON_HASH_UTIL_H_
#define COMMON_HASH_UTIL_H_

#include <cstddef>

#include <chrono>
#include <functional>
#include <tuple>
#include <type_traits>
#include <vector>

#include "common/tuple_util.h"

namespace fluent {
// The C++ standard library includes an `std::hash` struct template that can be
// used to hash a bunch of standard types. For example `std::hash<int>` is a
// struct which contains a call operator of type `std::size_t operator()(int
// x)` (or something equivalent) which can be used to hash integers like this
// `std::hash<int>()(42)`.
//
// There are a couple of types that `std::hash` does not support that we want
// to hash. For example `std::hash` cannot be used to hash tuples. The `Hash`
// struct template is an extension of `std::hash`. It supports everything that
// `std::hash` does, but also supports a couple other types (like tuples).
template <typename K>
struct Hash {
  std::size_t operator()(const K& k) { return std::hash<K>()(k); }
};

template <typename Clock>
struct Hash<std::chrono::time_point<Clock>> {
  // The hash of a time point is the hash of the number of nanoseconds it
  // represents since the epoch.
  std::size_t operator()(const std::chrono::time_point<Clock>& time) {
    using namespace std::chrono;
    auto time_since_epoch = duration_cast<nanoseconds>(time.time_since_epoch());
    auto count = time_since_epoch.count();
    Hash<decltype(count)> h;
    return h(count);
  }
};

namespace detail {

// To hash a (possibly heterogeneous) sequence (e.g. vector, tuple) `[x1: T1,
// ..., xn: Tn]`, we first hash each element `xi` of the sequence using
// `Hash<Ti>`. We then combine the hashes by folding the `HashCombine` functor
// below over them.  The hash combining function was taken from a StackExchange
// question [1] which was in turn taken from a boost library.
//
// [1]: http://codereview.stackexchange.com/q/136770
struct HashCombine {
  template <typename T>
  std::size_t operator()(std::size_t acc, const T& x) {
    return acc ^ (Hash<T>()(x) * 0x9e3779b9 + (acc << 6) + (acc >> 2));
  }
};

}  // namespace detail

template <typename T>
struct Hash<std::vector<T>> {
  std::size_t operator()(const std::vector<T>& xs) {
    detail::HashCombine hash_combine;
    std::size_t acc = 0;
    for (const T& x : xs) {
      acc = hash_combine(acc, x);
    }
    return acc;
  }
};

template <typename... Ts>
struct Hash<std::tuple<Ts...>> {
  std::size_t operator()(const std::tuple<Ts...>& t) {
    detail::HashCombine hash_combine;
    return TupleFold(static_cast<std::size_t>(0), t, hash_combine);
  }
};

}  // namespace fluent

#endif  //  COMMON_HASH_UTIL_H_
