#include "ra/sum.h"

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

TEST(Sum, EmptySum) {
  std::vector<std::tuple<int>> xs = {};
  auto sum = ra::make_sum(ra::make_iterable(&xs));
  static_assert(
      std::is_same<decltype(sum)::column_types, TypeList<int>>::value,
      "");
  const std::set<std::tuple<int>> expected = {{0}};
  ExpectRngsEqual(sum.ToPhysical().ToRange(), expected);
}

TEST(Sum, SimpleSum) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto sum = ra::make_sum(ra::make_iterable(&xs));
  static_assert(
      std::is_same<decltype(sum)::column_types, TypeList<int>>::value,
      "");
  const std::set<std::tuple<int>> expected = {{6}};
  ExpectRngsEqual(sum.ToPhysical().ToRange(), expected);
}

TEST(Sum, SimplePipedSum) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto sum = ra::make_iterable(&xs) | ra::sum();
  static_assert(
      std::is_same<decltype(sum)::column_types, TypeList<int>>::value,
      "");
  const std::set<std::tuple<int>> expected = {{6}};
  ExpectRngsEqual(sum.ToPhysical().ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
