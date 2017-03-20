#include "common/string_util.h"

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

}  // namespace fluent
