#include "ra/group_by.h"

#include <cstddef>
#include <set>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/type_list.h"
#include "ra/aggregates.h"
#include "ra/iterable.h"
#include "testing/test_util.h"

namespace fluent {

TEST(GroupBy, SimpleSumCountAvg) {
  std::vector<std::tuple<int, int, int, int>> xs = {
      {1, 2, 9, 1}, {1, 3, 8, 2}, {1, 1, 0, 3}, {2, 1, 5, 3},
      {2, 2, 9, 4}, {2, 8, 3, 5}, {3, 3, 9, 4}, {3, 2, 3, 5},
      {3, 1, 1, 6}, {3, 0, 0, 7}, {3, 0, 0, 8},
  };
  std::vector<std::tuple<int, int, std::size_t, double>> expected = {
      {1, 6, 3, 2.0}, {2, 11, 3, 4.0}, {3, 6, 5, 6.0},
  };

  auto grouped = ra::make_iterable(&xs) |
                 ra::group_by<ra::Keys<0>, ra::agg::Sum<1>, ra::agg::Count<2>,
                              ra::agg::Avg<3>>();
  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<int, int, std::size_t, double>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
  EXPECT_EQ(grouped.ToDebugString(),
            "GroupBy<Keys<0>, Sum<1>, Count<2>, Avg<3>>(Iterable)");
}

TEST(GroupBy, EmptyKeys) {
  std::set<std::tuple<int>> xs = {{1}, {2}, {3}, {4}, {5}};
  std::set<std::tuple<std::size_t>> expected = {{5}};
  auto grouped =
      ra::make_iterable(&xs) | ra::group_by<ra::Keys<>, ra::agg::Count<0>>();
  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<std::size_t>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
}

TEST(GroupBy, EmptyEmptyKeys) {
  std::set<std::tuple<int>> xs = {};
  std::set<std::tuple<std::size_t>> expected = {};
  auto grouped =
      ra::make_iterable(&xs) | ra::group_by<ra::Keys<>, ra::agg::Count<0>>();
  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<std::size_t>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
