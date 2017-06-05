#include "ra/cross.h"

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

TEST(Cross, SimpleCross) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  std::vector<std::tuple<std::string>> ys = {{"a"}, {"b"}};

  auto crossed = ra::make_cross(ra::make_iterable(&xs), ra::make_iterable(&ys));
  static_assert(std::is_same<decltype(crossed)::column_types,
                             TypeList<int, std::string>>::value,
                "");
  std::vector<std::tuple<int, std::string>> expected = {
      {1, "a"}, {2, "a"}, {3, "a"}, {1, "b"}, {2, "b"}, {3, "b"},
  };
  ExpectRngsUnorderedEqual(crossed.ToPhysical().ToRange(), expected);
  EXPECT_EQ(crossed.ToDebugString(), "Cross(Iterable, Iterable)");
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
