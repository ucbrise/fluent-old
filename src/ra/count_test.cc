#include "ra/count.h"

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

TEST(Count, EmptyCount) {
  std::vector<std::tuple<int, int>> xs = {};
  auto count = ra::make_count(ra::make_iterable(&xs));
  static_assert(
      std::is_same<decltype(count)::column_types, TypeList<std::size_t>>::value,
      "");
  const std::set<std::tuple<std::size_t>> expected = {{0}};
  ExpectRngsEqual(count.ToPhysical().ToRange(), expected);
  EXPECT_EQ(count.ToDebugString(), "Count(Iterable)");
}

TEST(Count, SimpleCount) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto count = ra::make_count(ra::make_iterable(&xs));
  static_assert(
      std::is_same<decltype(count)::column_types, TypeList<std::size_t>>::value,
      "");
  const std::set<std::tuple<std::size_t>> expected = {{3}};
  ExpectRngsEqual(count.ToPhysical().ToRange(), expected);
}

TEST(Count, SimplePipedCount) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto count = ra::make_iterable(&xs) | ra::count();
  static_assert(
      std::is_same<decltype(count)::column_types, TypeList<std::size_t>>::value,
      "");
  const std::set<std::tuple<std::size_t>> expected = {{3}};
  ExpectRngsEqual(count.ToPhysical().ToRange(), expected);
}

TEST(Count, ComplexPipedCount) {
  std::set<std::tuple<int, int>> xs = {{1, 1}, {2, 2}, {3, 3}};
  auto count = ra::make_iterable(&xs) | ra::count();
  static_assert(
      std::is_same<decltype(count)::column_types, TypeList<std::size_t>>::value,
      "");
  const std::set<std::tuple<std::size_t>> expected = {{3}};
  ExpectRngsEqual(count.ToPhysical().ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
