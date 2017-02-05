#include "example/count.h"

#include <cstddef>

#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

namespace example {

TEST(Count, SimpleRanges) {
  LOG(INFO) << "Running SimpleRanges test.";
  std::vector<int> zero = {};
  std::vector<int> one = {0};
  std::vector<int> five = {0, 0, 0, 0, 0};
  CHECK_EQ(five.size(), static_cast<std::size_t>(5)) << "5 != 5";

  EXPECT_EQ(0, Count(ranges::view::all(zero)));
  EXPECT_EQ(1, Count(ranges::view::all(one)));
  EXPECT_EQ(5, Count(ranges::view::all(five)));
}

TEST(Count, ComplexRanges) {
  std::vector<int> xs;
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(i, Count(ranges::view::all(xs)));
    xs.push_back(i);
  }
}

}  // namespace example

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
