#include "common/time_util.h"

#include <chrono>
#include <sstream>
#include <string>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace {

template <typename Clock>
std::string ToString(const std::chrono::time_point<Clock>& t) {
  std::ostringstream os;
  os << t;
  return os.str();
}

}  // namespace

TEST(TimeUtil, TimePointToString) {
  using time = std::chrono::time_point<std::chrono::system_clock>;
  using ns = std::chrono::nanoseconds;
  EXPECT_EQ("time(1 ns)", ToString(time(ns(1))));
  EXPECT_EQ("time(2 ns)", ToString(time(ns(2))));
  EXPECT_EQ("time(3 ns)", ToString(time(ns(3))));
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
