#include "common/string_util.h"

#include <algorithm>

namespace fluent {

std::string Join(const std::vector<std::string>& ss) {
  std::string s = "";
  if (ss.size() == 0) {
    return s;
  }
  for (std::size_t i = 0; i < ss.size() - 1; ++i) {
    s += ss[i] + ", ";
  }
  s += ss[ss.size() - 1];
  return s;
}

std::string CrunchWhitespace(std::string s) {
  // Replace newlines with spaces.
  std::replace(s.begin(), s.end(), '\n', ' ');

  // Destutter
  auto both_whitespace = [](char x, char y) { return x == y && x == ' '; };
  auto i = std::unique(s.begin(), s.end(), both_whitespace);
  s.erase(i, s.end());

  return s;
}

}  // namespace fluent
