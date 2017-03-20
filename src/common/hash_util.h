#ifndef COMMON_HASH_UTIL_H_
#define COMMON_HASH_UTIL_H_

#include <cstddef>

#include <functional>
#include <tuple>
#include <type_traits>

#include "common/tuple_util.h"

namespace fluent {

template <typename K>
struct Hash {
  std::size_t operator()(const K& k) { return std::hash<K>()(k); }
};

template <typename... Ts>
struct Hash<std::tuple<Ts...>> {
  std::size_t operator()(const std::tuple<Ts...>& k) {
    // This hash_combine function was taken from a StackExchange question [1]
    // which was in turn taken from a boost library.
    //
    // [1]: http://codereview.stackexchange.com/q/136770
    auto hash_combine = [](const std::size_t& acc, const auto& x) {
      using x_type = typename std::decay<decltype(x)>::type;
      std::size_t hash = Hash<x_type>()(x);
      return acc ^ (hash * 0x9e3779b9 + (acc << 6) + (acc >> 2));
    };

    return TupleFold(static_cast<std::size_t>(0), k, hash_combine);
  }
};

}  // namespace fluent

#endif  //  COMMON_HASH_UTIL_H_
