#include "fluent/scratch.h"

#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace fluent {

using ::testing::UnorderedElementsAreArray;

TEST(Scratch, SimpleMerge) {
  Scratch<int, int> s("s");
  std::set<std::tuple<int, int>> s1 = {{1, 1}, {2, 2}};
  std::set<std::tuple<int, int>> s2 = {{2, 2}, {3, 3}};
  std::set<std::tuple<int, int>> s3 = {{1, 1}, {2, 2}, {3, 3}};

  s.Merge(ra::make_iterable(&s1));
  EXPECT_THAT(s.Get(), testing::UnorderedElementsAreArray(s1));
  s.Merge(ra::make_iterable(&s2));
  EXPECT_THAT(s.Get(), testing::UnorderedElementsAreArray(s3));
}

TEST(Scratch, SimpleSetMerge) {
  Scratch<int, int> s("s");
  std::set<std::tuple<int, int>> s1 = {{1, 1}, {2, 2}};
  std::set<std::tuple<int, int>> s2 = {{2, 2}, {3, 3}};
  std::set<std::tuple<int, int>> s3 = {{1, 1}, {2, 2}, {3, 3}};

  s.Merge(s1);
  EXPECT_THAT(s.Get(), testing::UnorderedElementsAreArray(s1));
  s.Merge(s2);
  EXPECT_THAT(s.Get(), testing::UnorderedElementsAreArray(s3));
}

TEST(Scratch, SelfMerge) {
  Scratch<int, int> s("s");
  std::set<std::tuple<int, int>> ts = {{1, 1}, {2, 2}};

  s.Merge(ra::make_iterable(&ts));
  EXPECT_THAT(s.Get(), testing::UnorderedElementsAreArray(ts));
  s.Merge(ra::make_iterable(&s.Get()));
  EXPECT_THAT(s.Get(), testing::UnorderedElementsAreArray(ts));
}

TEST(Scratch, TickClearsScratches) {
  Scratch<int, int> s("s");
  std::set<std::tuple<int, int>> empty = {};
  std::set<std::tuple<int, int>> ts = {{1, 1}, {2, 2}};

  s.Merge(ra::make_iterable(&ts));
  EXPECT_THAT(s.Get(), testing::UnorderedElementsAreArray(ts));
  s.Tick();
  EXPECT_THAT(s.Get(), testing::UnorderedElementsAreArray(empty));
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
