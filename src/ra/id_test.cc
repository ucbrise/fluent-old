#include "ra/id.h"

#include <set>
#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/hash_util.h"
#include "common/type_list.h"
#include "ra/iterable.h"
#include "ra/lineaged_tuple.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Id, EmptyId) {
  std::set<std::tuple<int>> xs = {};
  auto id = ra::make_iterable("xs", &xs) | ra::id();
  std::set<ra::LineagedTuple<int>> expected = {};

  using column_types = decltype(id)::column_types;
  static_assert(std::is_same<column_types, TypeList<int>>::value, "");
  ExpectRngsUnorderedEqual(id.ToPhysical().ToRange(), expected);
  EXPECT_EQ(id.ToDebugString(), "Id(xs)");
}

TEST(Id, SimplePipedId) {
  Hash<std::tuple<int>> hash;
  std::tuple<int> t0 = {1};
  std::tuple<int> t1 = {2};
  std::tuple<int> t2 = {3};
  std::set<std::tuple<int>> xs = {t0, t1, t2};
  auto id = ra::make_iterable("xs", &xs) | ra::id();

  std::set<ra::LineagedTuple<int>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}}, t0),
      ra::make_lineaged_tuple({{"xs", hash(t1)}}, t1),
      ra::make_lineaged_tuple({{"xs", hash(t2)}}, t2),
  };

  using column_types = decltype(id)::column_types;
  static_assert(std::is_same<column_types, TypeList<int>>::value, "");
  ExpectRngsUnorderedEqual(id.ToPhysical().ToRange(), expected);
  EXPECT_EQ(id.ToDebugString(), "Id(xs)");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
