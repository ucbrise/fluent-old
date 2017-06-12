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
  using ms = std::chrono::microseconds;
  EXPECT_EQ("time(1000 ns)", ToString(time(ms(1))));
  EXPECT_EQ("time(2000 ns)", ToString(time(ms(2))));
  EXPECT_EQ("time(3000 ns)", ToString(time(ms(3))));
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
