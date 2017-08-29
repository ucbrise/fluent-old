#include "testing/mock_clock.h"

#include <chrono>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace testing {

TEST(MockClock, SimpleTest) {
  std::chrono::time_point<MockClock> epoch;
  std::chrono::time_point<MockClock> t0 = MockClock::now();
  MockClock::Advance(std::chrono::seconds(1));
  std::chrono::time_point<MockClock> t1 = MockClock::now();
  MockClock::Advance(std::chrono::seconds(1));
  std::chrono::time_point<MockClock> t2 = MockClock::now();
  MockClock::Reset();
  std::chrono::time_point<MockClock> t3 = MockClock::now();
  MockClock::Advance(std::chrono::seconds(1));
  std::chrono::time_point<MockClock> t4 = MockClock::now();
  MockClock::Advance(std::chrono::seconds(1));
  std::chrono::time_point<MockClock> t5 = MockClock::now();

  EXPECT_EQ(epoch, t0);
  EXPECT_EQ(epoch + std::chrono::seconds(1), t1);
  EXPECT_EQ(epoch + std::chrono::seconds(2), t2);
  EXPECT_EQ(epoch, t3);
  EXPECT_EQ(epoch + std::chrono::seconds(1), t4);
  EXPECT_EQ(epoch + std::chrono::seconds(2), t5);
}

}  // namespace testing
}  // namespace fluent

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
