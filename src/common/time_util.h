#ifndef COMMON_TIME_UTIL_H_
#define COMMON_TIME_UTIL_H_

#include <chrono>
#include <iostream>
#include <string>

namespace fluent {

template <typename Clock>
std::ostream& operator<<(std::ostream& out,
                         const std::chrono::time_point<Clock>& t) {
  using namespace std::chrono;
  nanoseconds ns = duration_cast<nanoseconds>(t.time_since_epoch());
  out << "time(" << ns.count() << " ns)";
  return out;
}

}  // namespace fluent

#endif  //  COMMON_TIME_UTIL_H_
