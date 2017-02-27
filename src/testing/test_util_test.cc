#include "testing/test_util.h"

#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

namespace fluent {

// A test to test testing utilities. Woah.

TEST(ExpectRngsEqual, SimpleTest) {
  {
    std::vector<int> xs = {1, 2, 3, 4};
    std::vector<int> ys = {1, 2, 3, 4};
    ExpectRngsEqual(ranges::view::all(xs), ranges::view::all(ys));
  }

  {
    std::vector<int> xs = {1, 2, 3, 4};
    std::vector<int> ys = {4, 8, 12, 16};
    auto rng_a = xs | ranges::view::transform([](int x) { return x * 2; });
    auto rng_b = ys | ranges::view::transform([](int y) { return y / 2; });
    ExpectRngsEqual(rng_a, rng_b);
  }
}

TEST(ExpectRngsUnorderedEqual, SimpleTest) {
  {
    std::vector<int> xs = {1, 2, 3};
    ExpectRngsUnorderedEqual(ranges::view::all(xs), xs);
  }
  {
    std::vector<int> xs = {1, 2, 3};
    std::vector<int> ys = {3, 2, 1};
    ExpectRngsUnorderedEqual(ranges::view::all(xs), ys);
  }
  {
    std::vector<int> xs = {1, 2, 3};
    std::vector<int> ys = {3, 2, 1};
    ExpectRngsUnorderedEqual(ranges::view::all(xs), ys);
  }
  {
    std::vector<int> xs = {1, 2, 3, 3};
    std::vector<int> ys = {3, 2, 1, 3};
    ExpectRngsUnorderedEqual(ranges::view::all(xs), ys);
  }
}

}  // namespace example

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
