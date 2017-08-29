#ifndef COMMON_COLLECTION_UTIL_H_
#define COMMON_COLLECTION_UTIL_H_

#include <ostream>
#include <set>
#include <vector>

namespace fluent {
namespace common {

// Pretty print an std::set (e.g. {1, 2, 3}).
template <typename... Ts>
std::ostream& operator<<(std::ostream& out, const std::set<Ts...>& xs) {
  out << "{";
  std::size_t i = 0;
  for (const auto& x : xs) {
    if (i == xs.size() - 1) {
      out << x;
    } else {
      out << x << ", ";
    }
    ++i;
  }
  out << "}";
  return out;
}

// Pretty print an std::vector (e.g. [1, 2, 3]).
template <typename... Ts>
std::ostream& operator<<(std::ostream& out, const std::vector<Ts...>& xs) {
  out << "[";
  std::size_t i = 0;
  for (const auto& x : xs) {
    if (i == xs.size() - 1) {
      out << x;
    } else {
      out << x << ", ";
    }
    ++i;
  }
  out << "]";
  return out;
}

}  // namespace common
}  // namespace fluent

#endif  //  COMMON_COLLECTION_UTIL_H_
