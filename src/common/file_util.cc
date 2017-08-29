#include "common/file_util.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "fmt/format.h"

#include "common/status.h"
#include "common/status_or.h"

namespace fluent {
namespace common {

// https://stackoverflow.com/a/2602258/3187068
StatusOr<std::string> Slurp(const std::string& filename) {
  std::ifstream t(filename);
  if (!t.is_open()) {
    const std::string err = fmt::format("File '{}' not found.", filename);
    return Status(ErrorCode::NOT_FOUND, err);
  }
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

}  // namespace common
}  // namespace fluent
