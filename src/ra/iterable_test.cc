#include "ra/iterable.h"

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/type_list.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Relalg, EmptyIterable) {
  std::vector<std::tuple<int>> xs = {};
  auto iter = ra::make_iterable(&xs);
  static_assert(
      std::is_same<decltype(iter)::column_types, TypeList<int>>::value, "");
  ExpectRngsEqual(iter.ToPhysical().ToRange(), ranges::view::all(xs));
  EXPECT_EQ(iter.ToDebugString(), "Iterable");
}

TEST(Relalg, SimpleIterable) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto iter = ra::make_iterable(&xs);
  static_assert(
      std::is_same<decltype(iter)::column_types, TypeList<int>>::value, "");
  ExpectRngsEqual(iter.ToPhysical().ToRange(), ranges::view::all(xs));
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
