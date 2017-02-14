#include "ra/groupby.h"
#include "ra/groupsum.h"

#include <tuple>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/iterable.h"
#include "ra/map.h"
#include "testing/test_util.h"

namespace fluent {

TEST(GroupBy, SingleColumnGroupSum) {
  std::vector<std::tuple<int, int>> xs = {{6, 1}, {4, 3}, {6, 5}, {4, 1}};
  {
    auto groupby = ra::make_iterable(&xs) | ra::groupby<0>()
                                          | ra::groupsum<1>();
    std::vector<std::tuple<int>> expected = {{4}, {6}};
    ExpectRngsEqual(groupby.ToPhysical().ToRange(),
                    ranges::view::all(expected));
  }
}

TEST(GroupBy, MultiColumnGroupSum) {
  std::vector<std::tuple<int, int, int>> xs = {{6, 1, 3}, {4, 3, 5}, {6, 1, 6}, {4, 3, 2}};
  {
    auto groupby = ra::make_iterable(&xs) | ra::groupby<0, 1>()
                                          | ra::groupsum<2>();
    std::vector<std::tuple<int>> expected = {{7}, {9}};
    ExpectRngsEqual(groupby.ToPhysical().ToRange(),
                    ranges::view::all(expected));
  }
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
