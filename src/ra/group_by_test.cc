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

TEST(GroupBy, SimpleCount) {
  using tuple = std::tuple<int, int>;
  Hash<tuple> hash;
  tuple t0{0, 0};
  tuple t1{0, 1};
  tuple t2{1, 2};
  tuple t3{1, 3};
  tuple t4{1, 4};
  tuple t5{2, 5};
  std::set<tuple> xs = {t0, t1, t2, t3, t4, t5};

  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<0>, ra::agg::Count<1>>();

  using GroupedTuple = std::tuple<int, std::size_t>;
  std::set<ra::LineagedTuple<int, std::size_t>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}, {"xs", hash(t1)}},
                              GroupedTuple{0, 2}),
      ra::make_lineaged_tuple(
          {{"xs", hash(t2)}, {"xs", hash(t3)}, {"xs", hash(t4)}},
          GroupedTuple{1, 3}),
      ra::make_lineaged_tuple({{"xs", hash(t5)}}, GroupedTuple{2, 1}),
  };

  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<int, std::size_t>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
  EXPECT_EQ(grouped.ToDebugString(), "GroupBy<Keys<0>, Count<1>>(xs)");
}

TEST(GroupBy, VariadicCount) {
  using tuple = std::tuple<int, int, int, int>;
  Hash<tuple> hash;
  tuple t0{0, 0, 0, 0};
  tuple t1{0, 1, 1, 1};
  tuple t2{1, 2, 2, 2};
  tuple t3{1, 3, 3, 3};
  tuple t4{1, 4, 4, 4};
  tuple t5{2, 5, 5, 5};
  std::set<tuple> xs = {t0, t1, t2, t3, t4, t5};

  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<0>, ra::agg::Count<1, 2, 3, 1, 2, 3>>();

  using GroupedTuple = std::tuple<int, std::size_t>;
  std::set<ra::LineagedTuple<int, std::size_t>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}, {"xs", hash(t1)}},
                              GroupedTuple{0, 2}),
      ra::make_lineaged_tuple(
          {{"xs", hash(t2)}, {"xs", hash(t3)}, {"xs", hash(t4)}},
          GroupedTuple{1, 3}),
      ra::make_lineaged_tuple({{"xs", hash(t5)}}, GroupedTuple{2, 1}),
  };

  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<int, std::size_t>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
  EXPECT_EQ(grouped.ToDebugString(),
            "GroupBy<Keys<0>, Count<1, 2, 3, 1, 2, 3>>(xs)");
}

TEST(GroupBy, EmptyCount) {
  using tuple = std::tuple<int, int, int, int>;
  Hash<tuple> hash;
  tuple t0{0, 0, 0, 0};
  tuple t1{0, 1, 1, 1};
  tuple t2{1, 2, 2, 2};
  tuple t3{1, 3, 3, 3};
  tuple t4{1, 4, 4, 4};
  tuple t5{2, 5, 5, 5};
  std::set<tuple> xs = {t0, t1, t2, t3, t4, t5};

  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<0>, ra::agg::Count<>>();

  using GroupedTuple = std::tuple<int, std::size_t>;
  std::set<ra::LineagedTuple<int, std::size_t>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}, {"xs", hash(t1)}},
                              GroupedTuple{0, 2}),
      ra::make_lineaged_tuple(
          {{"xs", hash(t2)}, {"xs", hash(t3)}, {"xs", hash(t4)}},
          GroupedTuple{1, 3}),
      ra::make_lineaged_tuple({{"xs", hash(t5)}}, GroupedTuple{2, 1}),
  };

  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<int, std::size_t>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
  EXPECT_EQ(grouped.ToDebugString(), "GroupBy<Keys<0>, Count<>>(xs)");
}

TEST(GroupBy, SimpleSum) {
  using tuple = std::tuple<int, int>;
  Hash<tuple> hash;
  tuple t0{0, 0};
  tuple t1{0, 1};
  tuple t2{1, 2};
  tuple t3{1, 3};
  tuple t4{1, 4};
  tuple t5{2, 5};
  std::set<tuple> xs = {t0, t1, t2, t3, t4, t5};

  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<0>, ra::agg::Sum<1>>();

  using GroupedTuple = std::tuple<int, int>;
  std::set<ra::LineagedTuple<int, int>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}, {"xs", hash(t1)}},
                              GroupedTuple{0, 1}),
      ra::make_lineaged_tuple(
          {{"xs", hash(t2)}, {"xs", hash(t3)}, {"xs", hash(t4)}},
          GroupedTuple{1, 9}),
      ra::make_lineaged_tuple({{"xs", hash(t5)}}, GroupedTuple{2, 5}),
  };

  static_assert(
      std::is_same<decltype(grouped)::column_types, TypeList<int, int>>::value,
      "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
  EXPECT_EQ(grouped.ToDebugString(), "GroupBy<Keys<0>, Sum<1>>(xs)");
}

TEST(GroupBy, VariadicSum) {
  using tuple = std::tuple<int, int, int, int>;
  Hash<tuple> hash;
  tuple t0{0, 0, 0, 0};
  tuple t1{0, 1, 1, 1};
  tuple t2{1, 2, 2, 2};
  tuple t3{1, 3, 3, 3};
  tuple t4{1, 4, 4, 4};
  tuple t5{2, 5, 5, 5};
  std::set<tuple> xs = {t0, t1, t2, t3, t4, t5};

  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<0>, ra::agg::Sum<1, 2, 3, 1, 2, 3>>();

  using GroupedTuple = std::tuple<int, int>;
  std::set<ra::LineagedTuple<int, int>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}, {"xs", hash(t1)}},
                              GroupedTuple{0, 6}),
      ra::make_lineaged_tuple(
          {{"xs", hash(t2)}, {"xs", hash(t3)}, {"xs", hash(t4)}},
          GroupedTuple{1, 54}),
      ra::make_lineaged_tuple({{"xs", hash(t5)}}, GroupedTuple{2, 30}),
  };

  static_assert(
      std::is_same<decltype(grouped)::column_types, TypeList<int, int>>::value,
      "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
  EXPECT_EQ(grouped.ToDebugString(),
            "GroupBy<Keys<0>, Sum<1, 2, 3, 1, 2, 3>>(xs)");
}

TEST(GroupBy, SimpleAvg) {
  using tuple = std::tuple<int, int>;
  Hash<tuple> hash;
  tuple t0{0, 0};
  tuple t1{0, 1};
  tuple t2{1, 2};
  tuple t3{1, 3};
  tuple t4{1, 4};
  tuple t5{2, 5};
  std::set<tuple> xs = {t0, t1, t2, t3, t4, t5};

  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<0>, ra::agg::Avg<1>>();

  using GroupedTuple = std::tuple<int, double>;
  std::set<ra::LineagedTuple<int, double>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}, {"xs", hash(t1)}},
                              GroupedTuple{0, 0.5}),
      ra::make_lineaged_tuple(
          {{"xs", hash(t2)}, {"xs", hash(t3)}, {"xs", hash(t4)}},
          GroupedTuple{1, 3}),
      ra::make_lineaged_tuple({{"xs", hash(t5)}}, GroupedTuple{2, 5}),
  };

  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<int, double>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
  EXPECT_EQ(grouped.ToDebugString(), "GroupBy<Keys<0>, Avg<1>>(xs)");
}

TEST(GroupBy, VariadicAvg) {
  using tuple = std::tuple<int, int, int>;
  Hash<tuple> hash;
  tuple t0{0, 0, 2};
  tuple t1{0, 1, 3};
  tuple t2{1, 2, 5};
  tuple t3{1, 3, 6};
  tuple t4{1, 4, 7};
  tuple t5{2, 5, 5};
  std::set<tuple> xs = {t0, t1, t2, t3, t4, t5};

  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<0>, ra::agg::Avg<1, 2>>();

  using GroupedTuple = std::tuple<int, double>;
  std::set<ra::LineagedTuple<int, double>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}, {"xs", hash(t1)}},
                              GroupedTuple{0, 1.5}),
      ra::make_lineaged_tuple(
          {{"xs", hash(t2)}, {"xs", hash(t3)}, {"xs", hash(t4)}},
          GroupedTuple{1, 4.5}),
      ra::make_lineaged_tuple({{"xs", hash(t5)}}, GroupedTuple{2, 5}),
  };

  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<int, double>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
  EXPECT_EQ(grouped.ToDebugString(), "GroupBy<Keys<0>, Avg<1, 2>>(xs)");
}

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

TEST(GroupBy, EmptyKeys) {
  Hash<std::tuple<int>> hash;
  std::tuple<int> x0{1};
  std::tuple<int> x1{2};
  std::tuple<int> x2{3};
  std::tuple<int> x3{4};
  std::tuple<int> x4{5};
  std::set<std::tuple<int>> xs = {x0, x1, x2, x3, x4};
  std::set<ra::LineagedTuple<std::size_t>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(x0)},
                               {"xs", hash(x1)},
                               {"xs", hash(x2)},
                               {"xs", hash(x3)},
                               {"xs", hash(x4)}},
                              std::tuple<std::size_t>(5))};
  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<>, ra::agg::Count<0>>();
  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<std::size_t>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
  EXPECT_EQ(grouped.ToDebugString(), "GroupBy<Keys<>, Count<0>>(xs)");
}

TEST(GroupBy, EmptyEmptyKeys) {
  std::set<std::tuple<int>> xs = {};
  std::set<ra::LineagedTuple<std::size_t>> expected = {};
  auto grouped = ra::make_iterable("xs", &xs) |
                 ra::group_by<ra::Keys<>, ra::agg::Count<0>>();
  static_assert(std::is_same<decltype(grouped)::column_types,
                             TypeList<std::size_t>>::value,
                "");
  ExpectRngsUnorderedEqual(grouped.ToPhysical().ToRange(), expected);
  EXPECT_EQ(grouped.ToDebugString(), "GroupBy<Keys<>, Count<0>>(xs)");
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
