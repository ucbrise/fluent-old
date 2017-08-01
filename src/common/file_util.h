#ifndef COMMON_FILE_UTIL_H_
#define COMMON_FILE_UTIL_H_

#include <string>

#include "common/status_or.h"

namespace fluent {
namespace common {

StatusOr<std::string> Slurp(const std::string& filename);

}  // namespace common
}  // namespace fluent

#endif  //  COMMON_FILE_UTIL_H_
