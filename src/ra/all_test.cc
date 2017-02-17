#include <tuple>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/all.h"
#include "testing/test_util.h"

namespace fluent {

TEST(All, SimpleAll) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};

  // clang-format off
  auto relalg = ra::make_iterable(&xs)
    | ra::map([](const auto& t) {
        int x = std::get<0>(t);
        return std::tuple<int, int>(x, x);
      })
    | ra::filter([](const auto& t) { return std::get<0>(t) % 2 == 1; })
    | ra::map([](const auto& t) {
        return std::make_tuple(std::get<0>(t) + std::get<1>(t));
      });
  // clang-format on

  static_assert(
      std::is_same<decltype(relalg)::column_types, ra::TypeList<int>>::value,
      "");
  std::vector<std::tuple<int>> expected = {{2}, {6}};
  ExpectRngsEqual(relalg.ToPhysical().ToRange(), ranges::view::all(expected));
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
