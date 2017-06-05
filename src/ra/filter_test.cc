#include "ra/filter.h"

#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/iterable.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Filter, SimpleFilter) {
  Hash<std::tuple<int>> hash;
  std::tuple<int> t0 = {1};
  std::tuple<int> t1 = {2};
  std::tuple<int> t2 = {3};
  std::set<std::tuple<int>> xs = {t0, t1, t2};
  auto filter = ra::make_filter(
      ra::make_iterable("xs", &xs),
      [](const std::tuple<int>& t) { return std::get<0>(t) % 2 == 1; });

  std::set<ra::LineagedTuple<int>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}}, t0),
      ra::make_lineaged_tuple({{"xs", hash(t2)}}, t2),
  };

  static_assert(
      std::is_same<decltype(filter)::column_types, TypeList<int>>::value, "");
  ExpectRngsUnorderedEqual(filter.ToPhysical().ToRange(), expected);
  EXPECT_EQ(filter.ToDebugString(), "Filter(xs)");
}

TEST(Filter, SimplePipedFilter) {
  Hash<std::tuple<int>> hash;
  std::tuple<int> t0 = {1};
  std::tuple<int> t1 = {2};
  std::tuple<int> t2 = {3};
  std::set<std::tuple<int>> xs = {t0, t1, t2};
  auto filter =
      ra::make_iterable("xs", &xs) | ra::filter([](const std::tuple<int>& t) {
        return std::get<0>(t) % 2 == 1;
      });

  std::set<ra::LineagedTuple<int>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}}, t0),
      ra::make_lineaged_tuple({{"xs", hash(t2)}}, t2),
  };

  static_assert(
      std::is_same<decltype(filter)::column_types, TypeList<int>>::value, "");
  ExpectRngsUnorderedEqual(filter.ToPhysical().ToRange(), expected);
  EXPECT_EQ(filter.ToDebugString(), "Filter(xs)");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
