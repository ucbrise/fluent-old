#include "relalg/project.h"

#include <tuple>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "testing/test_util.h"

namespace fluent {

TEST(Project, Id) {
  std::vector<int> xs = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  auto projected = xs | project([](int x) { return x; });
  ExpectRngsEqual(projected, ranges::view::all(xs));
}

TEST(Project, PlusOne) {
  std::vector<int> xs = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector<int> plus_one = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  auto projected = xs | project([](int x) { return x + 1; });
  ExpectRngsEqual(projected, ranges::view::all(plus_one));
}

TEST(Project, TupleProjection) {
  std::vector<std::tuple<int, bool, float>> xs = {
      {0, true, 0.0}, {1, false, 1.0}, {2, true, 2.0}, {3, false, 3.0},
      {4, true, 4.0}, {5, false, 5.0}, {6, true, 6.0}, {7, false, 7.0},
      {8, true, 8.0}, {9, false, 9.0}};
  std::vector<int> first = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector<bool> second = {true,  false, true,  false, true,
                              false, true,  false, true,  false};
  std::vector<float> third = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
  std::vector<std::tuple<float, bool, int>> reversed = {
      {0.0, true, 0}, {1.0, false, 1}, {2.0, true, 2}, {3.0, false, 3},
      {4.0, true, 4}, {5.0, false, 5}, {6.0, true, 6}, {7.0, false, 7},
      {8.0, true, 8}, {9.0, false, 9}};

  {
    auto projected = xs | project([](const auto& t) { return std::get<0>(t); });
    ExpectRngsEqual(projected, ranges::view::all(first));
  }

  {
    auto projected = xs | project([](const auto& t) { return std::get<1>(t); });
    ExpectRngsEqual(projected, ranges::view::all(second));
  }

  {
    auto projected = xs | project([](const auto& t) { return std::get<2>(t); });
    ExpectRngsEqual(projected, ranges::view::all(third));
  }

  {
    auto projected = xs | project([](const auto& t) {
                       return std::make_tuple(std::get<2>(t), std::get<1>(t),
                                              std::get<0>(t));
                     });
    ExpectRngsEqual(projected, ranges::view::all(reversed));
  }
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
