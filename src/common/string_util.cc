#include "common/string_util.h"

#include <algorithm>
#include <iterator>
#include <string>

namespace fluent {
namespace common {

std::vector<std::string> Split(const std::string& s) {
  // http://stackoverflow.com/a/5607650/3187068
  std::stringstream ss(s);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  return {begin, end};
}

std::string ToLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  return s;
}

std::string ToUpper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
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

}  // namespace common
}  // namespace fluent
