#include "ra/count.h"

#include <iostream>

#include <cstddef>
#include <set>
#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/hash_util.h"
#include "common/type_list.h"
#include "ra/iterable.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Count, EmptyCount) {
  std::set<std::tuple<int, int>> xs = {};
  auto count = ra::make_count(ra::make_iterable("xs", &xs));
  std::set<ra::LineagedTuple<std::size_t>> expected = {
      ra::make_lineaged_tuple({}, std::tuple<std::size_t>{0})};

  static_assert(
      std::is_same<decltype(count)::column_types, TypeList<std::size_t>>::value,
      "");
  ExpectRngsUnorderedEqual(count.ToPhysical().ToRange(), expected);
  EXPECT_EQ(count.ToDebugString(), "Count(xs)");
}

TEST(Count, SimpleCount) {
  Hash<std::tuple<int>> hash;
  std::tuple<int> t1 = {1};
  std::tuple<int> t2 = {2};
  std::tuple<int> t3 = {3};
  std::set<std::tuple<int>> xs = {t1, t2, t3};
  auto count = ra::make_count(ra::make_iterable("xs", &xs));

  std::set<ra::LineagedTuple<std::size_t>> expected = {ra::make_lineaged_tuple(
      {{"xs", hash(t1)}, {"xs", hash(t2)}, {"xs", hash(t3)}},
      std::tuple<std::size_t>{3})};

  static_assert(
      std::is_same<decltype(count)::column_types, TypeList<std::size_t>>::value,
      "");
  ExpectRngsUnorderedEqual(count.ToPhysical().ToRange(), expected);
  EXPECT_EQ(count.ToDebugString(), "Count(xs)");
}

TEST(Count, SimplePipedCount) {
  Hash<std::tuple<int>> hash;
  std::tuple<int> t1 = {1};
  std::tuple<int> t2 = {2};
  std::tuple<int> t3 = {3};
  std::set<std::tuple<int>> xs = {t1, t2, t3};
  auto count = ra::make_iterable("xs", &xs) | ra::count();

  std::set<ra::LineagedTuple<std::size_t>> expected = {ra::make_lineaged_tuple(
      {{"xs", hash(t1)}, {"xs", hash(t2)}, {"xs", hash(t3)}},
      std::tuple<std::size_t>{3})};

  static_assert(
      std::is_same<decltype(count)::column_types, TypeList<std::size_t>>::value,
      "");
  ExpectRngsUnorderedEqual(count.ToPhysical().ToRange(), expected);
  EXPECT_EQ(count.ToDebugString(), "Count(xs)");
}

TEST(Count, ComplexPipedCount) {
  Hash<std::tuple<int, int>> hash;
  std::tuple<int, int> t1 = {1, 1};
  std::tuple<int, int> t2 = {2, 2};
  std::tuple<int, int> t3 = {3, 3};
  std::set<std::tuple<int, int>> xs = {t1, t2, t3};
  auto count = ra::make_iterable("xs", &xs) | ra::count();

  std::set<ra::LineagedTuple<std::size_t>> expected = {ra::make_lineaged_tuple(
      {{"xs", hash(t1)}, {"xs", hash(t2)}, {"xs", hash(t3)}},
      std::tuple<std::size_t>{3})};

  static_assert(
      std::is_same<decltype(count)::column_types, TypeList<std::size_t>>::value,
      "");
  ExpectRngsUnorderedEqual(count.ToPhysical().ToRange(), expected);
  EXPECT_EQ(count.ToDebugString(), "Count(xs)");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
