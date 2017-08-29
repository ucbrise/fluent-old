#include "common/rand_util.h"

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace fluent {
namespace common {

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Lt;

TEST(RandUtil, RandInt) {
  const int num_trials = 100;
  for (int low = 0; low < 5; ++low) {
    for (int high = low + 1; high < 5; ++high) {
      for (int i = 0; i < num_trials; ++i) {
        int x = RandInt(low, high);
        EXPECT_THAT(x, AllOf(Ge(low), Lt(high)));
      }
    }
  }
}

}  // namespace common
}  // namespace fluent

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
