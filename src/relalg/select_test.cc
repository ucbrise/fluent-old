#include "relalg/select.h"

#include <vector>

#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "testing/test_util.h"

namespace fluent {

TEST(Select, AllTrue) {
  std::vector<int> xs = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  auto selected = xs | select([](int) { return true; });
  ExpectRngsEqual(selected, ranges::view::all(xs));
}

TEST(Select, AllFalse) {
  std::vector<int> empty;
  std::vector<int> xs = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  auto selected = xs | select([](int) { return false; });
  ExpectRngsEqual(selected, ranges::view::all(empty));
}

TEST(Select, Evens) {
  std::vector<int> xs = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector<int> evens = {0, 2, 4, 6, 8};
  auto selected = xs | select([](int x) { return x % 2 == 0; });
  ExpectRngsEqual(selected, ranges::view::all(evens));
}

}  // namespace fluent

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
