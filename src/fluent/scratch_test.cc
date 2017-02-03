#include "fluent/scratch.h"

#include <set>
#include <tuple>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace fluent {

using ::testing::UnorderedElementsAreArray;

TEST(Scratch, SimpleTest) {
  Scratch<int, int, int> s("s");

  using Tuple = std::tuple<int, int, int>;
  using TupleSet = std::set<Tuple>;
  Tuple x1{1, 1, 1};
  Tuple x2{2, 2, 2};
  Tuple x3{3, 3, 3};

  EXPECT_THAT(s.Get(), UnorderedElementsAreArray(TupleSet{}));
  s.Add(x1);
  EXPECT_THAT(s.Get(), UnorderedElementsAreArray(TupleSet{x1}));
  s.Add(x2);
  EXPECT_THAT(s.Get(), UnorderedElementsAreArray(TupleSet{x1, x2}));
  s.Add(x3);
  EXPECT_THAT(s.Get(), UnorderedElementsAreArray(TupleSet{x1, x2, x3}));
  s.Tick();
  EXPECT_THAT(s.Get(), UnorderedElementsAreArray(TupleSet{}));
  s.Add(x1);
  EXPECT_THAT(s.Get(), UnorderedElementsAreArray(TupleSet{x1}));
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
