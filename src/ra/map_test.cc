#include "ra/map.h"

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/iterable.h"
#include "ra/type_list.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Map, SimpleMap) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto map = ra::make_map(ra::make_iterable(&xs), [](const std::tuple<int>& t) {
    int x = std::get<0>(t);
    return std::tuple<int, int>(x, x);
  });
  static_assert(
      std::is_same<decltype(map)::column_types, ra::TypeList<int, int>>::value,
      "");
  std::vector<std::tuple<int, int>> expected = {{1, 1}, {2, 2}, {3, 3}};
  ExpectRngsEqual(map.ToPhysical().ToRange(), ranges::view::all(expected));
}

TEST(Map, SimplePipedMap) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto map = ra::make_iterable(&xs) | ra::map([](const std::tuple<int>& t) {
               int x = std::get<0>(t);
               return std::tuple<int, int>(x, x);
             });
  static_assert(
      std::is_same<decltype(map)::column_types, ra::TypeList<int, int>>::value,
      "");
  std::vector<std::tuple<int, int>> expected = {{1, 1}, {2, 2}, {3, 3}};
  ExpectRngsEqual(map.ToPhysical().ToRange(), ranges::view::all(expected));
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
