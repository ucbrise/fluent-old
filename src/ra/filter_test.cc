#include "ra/filter.h"

#include <tuple>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/iterable.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Filter, SimpleFilter) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto filter = ra::make_filter(
      ra::make_iterable(&xs),
      [](const std::tuple<int>& t) { return std::get<0>(t) % 2 == 1; });
  static_assert(
      std::is_same<decltype(filter)::column_types, ra::TypeList<int>>::value,
      "");
  std::vector<std::tuple<int>> expected = {{1}, {3}};
  ExpectRngsEqual(filter.ToPhysical().ToRange(), ranges::view::all(expected));
}

TEST(Filter, SimplePipedFilter) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto filter =
      ra::make_iterable(&xs) | ra::filter([](const std::tuple<int>& t) {
        return std::get<0>(t) % 2 == 1;
      });
  static_assert(
      std::is_same<decltype(filter)::column_types, ra::TypeList<int>>::value,
      "");
  std::vector<std::tuple<int>> expected = {{1}, {3}};
  ExpectRngsEqual(filter.ToPhysical().ToRange(), ranges::view::all(expected));
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
