#ifndef COMMON_STRING_UTIL_H_
#define COMMON_STRING_UTIL_H_

#include <cstddef>

#include <array>
#include <string>
#include <vector>

namespace fluent {
namespace detail {

template <typename Iterator>
std::string JoinIterators(Iterator begin, Iterator end) {
  std::string s = "";
  while (begin != end) {
    s += *begin;
    begin++;
    if (begin != end) {
      s += ", ";
    }
  }
  return s;
}

}  // namespace detail

// Join() = ""
// Join(1) = "1"
// Join(1, "2") = "1, 2"
// Join(1, "2", '3') = "1, 2, 3"
// Join(std::vector<std::string>{"1", "2", "3"}) = "1, 2, 3"
// Join(std::array<std::string, 3>{"1", "2", "3"}) = "1, 2, 3"
inline std::string Join() { return ""; }

inline std::string Join(const std::string& x) { return x; }

template <typename T>
std::string Join(const T& x) {
  return std::to_string(x);
}

template <typename... Ts>
std::string Join(const std::string& x, const Ts&... xs) {
  return x + ", " + Join(xs...);
}

template <typename T, typename... Ts>
std::string Join(const T& x, const Ts&... xs) {
  return std::to_string(x) + ", " + Join(xs...);
}

inline std::string Join(const std::vector<std::string>& ss) {
  return detail::JoinIterators(ss.cbegin(), ss.cend());
}

template <std::size_t N>
std::string Join(const std::array<std::string, N>& ss) {
  return detail::JoinIterators(ss.cbegin(), ss.cend());
}

// CrunchWhitespace converts newlines to spaces and then destutters spaces.
//
// CrunchWhitespace("")       == ""
// CrunchWhitespace(" ")      == " "
// CrunchWhitespace("  ")     == " "
// CrunchWhitespace("   ")    == " "
// CrunchWhitespace("a")      == "a"
// CrunchWhitespace(" a")     == " a"
// CrunchWhitespace(" a ")    == " a "
// CrunchWhitespace("  a ")   == " a "
// CrunchWhitespace("   a ")  == " a "
// CrunchWhitespace(" a  ")   == " a "
// CrunchWhitespace("  a  ")  == " a "
// CrunchWhitespace("   a  ") == " a "
// CrunchWhitespace("\n")     == " "
// CrunchWhitespace("\n\n")   == " "
// CrunchWhitespace("\n \n")  == " "
std::string CrunchWhitespace(std::string s);

}  // namespace fluent

#endif  //  COMMON_STRING_UTIL_H_
