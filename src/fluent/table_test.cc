#include "fluent/table.h"

#include <set>
#include <tuple>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace fluent {

using ::testing::UnorderedElementsAreArray;

TEST(Table, SimpleTest) {
  Table<int, int, int> t("t");

  using Tuple = std::tuple<int, int, int>;
  using TupleSet = std::set<Tuple>;
  Tuple x1{1, 1, 1};
  Tuple x2{2, 2, 2};
  Tuple x3{3, 3, 3};

  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(TupleSet{}));
  t.Add(x1);
  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(TupleSet{x1}));
  t.Add(x2);
  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(TupleSet{x1, x2}));
  t.Add(x3);
  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(TupleSet{x1, x2, x3}));
  t.Tick();
  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(TupleSet{x1, x2, x3}));
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
