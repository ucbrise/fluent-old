#ifndef COMMON_STRING_UTIL_H_
#define COMMON_STRING_UTIL_H_

#include <string>
#include <vector>

namespace fluent {

// Join() = ""
// Join(1) = "1"
// Join(1, "2") = "1, 2"
// Join(1, "2", '3') = "1, 2, 3"
// Join(std::vector<std::string>{"1", "2", "3"}) = "1, 2, 3"
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

std::string Join(const std::vector<std::string>& ss);

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
