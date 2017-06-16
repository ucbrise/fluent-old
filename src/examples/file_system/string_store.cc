#include "examples/file_system/string_store.h"

#include <cstddef>

void StringStore::Write(int off, const std::string& s) {
  s_.resize(std::max(s_.size(), off + s.size()), ' ');
  s_.replace(off, s.size(), s);
}

std::string StringStore::Read(int start, int stop) const {
  std::size_t start_sizet = std::max(start, 0);
  std::size_t stop_sizet = std::min(static_cast<std::size_t>(stop), s_.size());
  return s_.substr(start_sizet, stop_sizet - start_sizet);
}
