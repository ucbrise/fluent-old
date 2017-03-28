#include "ra/avg.h"

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/type_list.h"
#include "ra/iterable.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Avg, EmptyAvg) {
  std::vector<std::tuple<int>> xs = {};
  auto avg = ra::make_avg(ra::make_iterable(&xs));
  static_assert(
      std::is_same<decltype(avg)::column_types, TypeList<int>>::value,
      "");
  const std::set<std::tuple<int>> expected = {{0}};
  ExpectRngsEqual(avg.ToPhysical().ToRange(), expected);
}

TEST(Avg, SimpleAvg) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto avg = ra::make_avg(ra::make_iterable(&xs));
  static_assert(
      std::is_same<decltype(avg)::column_types, TypeList<int>>::value,
      "");
  const std::set<std::tuple<int>> expected = {{2}};
  ExpectRngsEqual(avg.ToPhysical().ToRange(), expected);
}

TEST(Avg, SimplePipedAvg) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto avg = ra::make_iterable(&xs) | ra::avg();
  static_assert(
      std::is_same<decltype(avg)::column_types, TypeList<int>>::value,
      "");
  const std::set<std::tuple<int>> expected = {{2}};
  ExpectRngsEqual(avg.ToPhysical().ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
