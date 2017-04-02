#include "ra/iterable.h"

#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/collection_util.h"
#include "common/hash_util.h"
#include "common/type_list.h"
#include "ra/lineaged_tuple.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Relalg, EmptyIterable) {
  std::set<std::tuple<int>> xs = {};
  auto iter = ra::make_iterable("xs", &xs);
  std::set<ra::LineagedTuple<int>> expected = {};

  static_assert(
      std::is_same<decltype(iter)::column_types, TypeList<int>>::value, "");
  ExpectRngsUnorderedEqual(iter.ToPhysical().ToRange(), expected);
  EXPECT_EQ(iter.ToDebugString(), "xs");
}

TEST(Relalg, SimpleIterable) {
  std::tuple<int> t0{1};
  std::tuple<int> t1{2};
  std::tuple<int> t3{3};
  std::set<std::tuple<int>> xs = {t0, t1, t3};

  Hash<std::tuple<int>> hash;
  std::set<ra::LineagedTuple<int>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}}, t0),
      ra::make_lineaged_tuple({{"xs", hash(t1)}}, t1),
      ra::make_lineaged_tuple({{"xs", hash(t3)}}, t3),
  };

  auto iter = ra::make_iterable("xs", &xs);
  static_assert(
      std::is_same<decltype(iter)::column_types, TypeList<int>>::value, "");
  ExpectRngsUnorderedEqual(iter.ToPhysical().ToRange(), expected);
  EXPECT_EQ(iter.ToDebugString(), "xs");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
