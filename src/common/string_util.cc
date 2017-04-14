#include "common/string_util.h"

#include <algorithm>

namespace fluent {

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
