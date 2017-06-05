#include "ra/group_by.h"

#include <cstddef>
#include <set>
#include <set>
#include <tuple>
#include <type_traits>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/hash_util.h"
#include "common/type_list.h"
#include "ra/aggregates.h"
#include "ra/iterable.h"
#include "testing/test_util.h"

namespace fluent {

TEST(GroupBy, SimpleSumCountAvg) {
  Hash<std::tuple<int, int, int, int>> hash;
  std::tuple<int, int, int, int> xs0 = {1, 2, 9, 1};
  std::tuple<int, int, int, int> xs1 = {1, 3, 8, 2};
  std::tuple<int, int, int, int> xs2 = {1, 1, 0, 3};
  std::tuple<int, int, int, int> xs3 = {2, 1, 5, 3};
  std::tuple<int, int, int, int> xs4 = {2, 2, 9, 4};
  std::tuple<int, int, int, int> xs5 = {2, 8, 3, 5};
  std::tuple<int, int, int, int> xs6 = {3, 3, 9, 4};
  std::tuple<int, int, int, int> xs7 = {3, 2, 3, 5};
  std::tuple<int, int, int, int> xs8 = {3, 1, 1, 6};
  std::tuple<int, int, int, int> xs9 = {3, 0, 0, 7};
  std::tuple<int, int, int, int> xs10 = {3, 0, 0, 8};
  std::set<std::tuple<int, int, int, int>> xs = {xs0, xs1, xs2, xs3, xs4, xs5,
                                                 xs6, xs7, xs8, xs9, xs10};

  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<0>, ra::agg::Sum<1>, ra::agg::Count<2>,
                              ra::agg::Avg<3>>();

  using GroupedTuple = std::tuple<int, int, std::size_t, double>;
  std::set<ra::LineagedTuple<int, int, std::size_t, double>> expected = {
      ra::make_lineaged_tuple(
          {{"xs", hash(xs0)}, {"xs", hash(xs1)}, {"xs", hash(xs2)}},
          GroupedTuple{1, 6, 3, 2.0}),
      ra::make_lineaged_tuple(
          {{"xs", hash(xs3)}, {"xs", hash(xs4)}, {"xs", hash(xs5)}},
          GroupedTuple{2, 11, 3, 4.0}),
      ra::make_lineaged_tuple({{"xs", hash(xs6)},
                               {"xs", hash(xs7)},
                               {"xs", hash(xs8)},
                               {"xs", hash(xs9)},
                               {"xs", hash(xs10)}},
                              GroupedTuple{3, 6, 5, 6.0}),
  };

  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<int, int, std::size_t, double>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
  EXPECT_EQ(grouped.ToDebugString(),
            "GroupBy<Keys<0>, Sum<1>, Count<2>, Avg<3>>(xs)");
}

#if 0
TEST(GroupBy, EmptyKeys) {
  std::set<std::tuple<int>> xs = {{1}, {2}, {3}, {4}, {5}};
  std::set<std::tuple<std::size_t>> expected = {{5}};
  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<>, ra::agg::Count<0>>();
  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<std::size_t>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
}

TEST(GroupBy, EmptyEmptyKeys) {
  std::set<std::tuple<int>> xs = {};
  std::set<std::tuple<std::size_t>> expected = {};
  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<>, ra::agg::Count<0>>();
  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<std::size_t>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
}
#endif

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
