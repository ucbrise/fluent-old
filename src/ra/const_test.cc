#include "ra/const.h"

#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/collection_util.h"
#include "common/type_list.h"
#include "ra/lineaged_tuple.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Relalg, EmptyConst) {
  std::set<std::tuple<int>> xs = {};
  auto cnst = ra::make_const("xs", &xs);
  std::set<ra::LineagedTuple<int>> expected = {};

  using column_types = decltype(cnst)::column_types;
  static_assert(std::is_same<column_types, TypeList<int>>::value, "");
  ExpectRngsUnorderedEqual(cnst.ToPhysical().ToRange(), expected);
  EXPECT_EQ(cnst.ToDebugString(), "xs");
}

TEST(Relalg, SimpleConst) {
  std::set<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto cnst = ra::make_const("xs", &xs);
  std::set<ra::LineagedTuple<int>> expected = {
      ra::make_lineaged_tuple({}, std::tuple<int>(1)),
      ra::make_lineaged_tuple({}, std::tuple<int>(2)),
      ra::make_lineaged_tuple({}, std::tuple<int>(3)),
  };

  using column_types = decltype(cnst)::column_types;
  static_assert(std::is_same<column_types, TypeList<int>>::value, "");
  ExpectRngsUnorderedEqual(cnst.ToPhysical().ToRange(), expected);
  EXPECT_EQ(cnst.ToDebugString(), "xs");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
