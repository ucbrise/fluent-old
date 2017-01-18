#include "relalg/cross.h"

#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "testing/test_util.h"

namespace fluent {

TEST(Cross, Simple) {
  std::vector<std::string> xs = {"a", "b"};
  std::vector<int> ys = {0, 1, 2};
  std::vector<std::pair<std::string, int>> zs = {{"a", 0}, {"a", 1}, {"a", 2},
                                                 {"b", 0}, {"b", 1}, {"b", 2}};
  auto crossed = cross(ranges::view::all(xs), ranges::view::all(ys));
  ExpectRngsEqual(crossed, ranges::view::all(zs));
}

}  // namespace fluent

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
