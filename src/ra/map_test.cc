#include "ra/map.h"

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
#include "ra/lineaged_tuple.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Map, EmptyMap) {
  std::set<std::tuple<int>> xs = {};
  auto map =
      ra::make_map(ra::make_iterable("xs", &xs), [](const std::tuple<int>& t) {
        int x = std::get<0>(t);
        return std::tuple<int, int>(x, x);
      });
  std::set<ra::LineagedTuple<int, int>> expected = {};

  static_assert(
      std::is_same<decltype(map)::column_types, TypeList<int, int>>::value, "");
  ExpectRngsUnorderedEqual(map.ToPhysical().ToRange(), expected);
  EXPECT_EQ(map.ToDebugString(), "Map(xs)");
}

TEST(Map, SimpleMap) {
  Hash<std::tuple<int>> hash;
  std::tuple<int> t0 = {1};
  std::tuple<int> t1 = {2};
  std::tuple<int> t2 = {3};
  std::set<std::tuple<int>> xs = {t0, t1, t2};
  auto map =
      ra::make_map(ra::make_iterable("xs", &xs), [](const std::tuple<int>& t) {
        int x = std::get<0>(t);
        return std::tuple<int, int>(x, x);
      });

  std::set<ra::LineagedTuple<int, int>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}}, std::make_tuple(1, 1)),
      ra::make_lineaged_tuple({{"xs", hash(t1)}}, std::make_tuple(2, 2)),
      ra::make_lineaged_tuple({{"xs", hash(t2)}}, std::make_tuple(3, 3)),
  };

  static_assert(
      std::is_same<decltype(map)::column_types, TypeList<int, int>>::value, "");
  ExpectRngsUnorderedEqual(map.ToPhysical().ToRange(), expected);
  EXPECT_EQ(map.ToDebugString(), "Map(xs)");
}

TEST(Map, SimplePipedMap) {
  Hash<std::tuple<int>> hash;
  std::tuple<int> t0 = {1};
  std::tuple<int> t1 = {2};
  std::tuple<int> t2 = {3};
  std::set<std::tuple<int>> xs = {t0, t1, t2};
  auto map =
      ra::make_iterable("xs", &xs) | ra::map([](const std::tuple<int>& t) {
        int x = std::get<0>(t);
        return std::tuple<int, int>(x, x);
      });

  std::set<ra::LineagedTuple<int, int>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}}, std::make_tuple(1, 1)),
      ra::make_lineaged_tuple({{"xs", hash(t1)}}, std::make_tuple(2, 2)),
      ra::make_lineaged_tuple({{"xs", hash(t2)}}, std::make_tuple(3, 3)),
  };

  static_assert(
      std::is_same<decltype(map)::column_types, TypeList<int, int>>::value, "");
  ExpectRngsUnorderedEqual(map.ToPhysical().ToRange(), expected);
  EXPECT_EQ(map.ToDebugString(), "Map(xs)");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
