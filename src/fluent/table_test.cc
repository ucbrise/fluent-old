#include "fluent/table.h"

#include <cstddef>

#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ra/all.h"

using ::testing::UnorderedElementsAreArray;

namespace fluent {
namespace {

template <typename A, typename B, typename C>
const A& fst(const std::tuple<A, B, C>& t) {
  return std::get<0>(t);
}

template <typename A, typename B, typename C>
const B& snd(const std::tuple<A, B, C>& t) {
  return std::get<1>(t);
}

template <typename A, typename B, typename C>
const C& thd(const std::tuple<A, B, C>& t) {
  return std::get<2>(t);
}

}  // namespace

TEST(Table, SimpleMerge) {
  Table<int, int> t("t");
  std::set<std::tuple<int, int>> empty;
  std::set<std::tuple<int, int>> s1 = {{1, 1}, {2, 2}};
  std::set<std::tuple<int, int>> s2 = {{2, 2}, {3, 3}};
  std::set<std::tuple<int, int>> s3 = {{1, 1}, {2, 2}, {3, 3}};

  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.Merge(s1);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s1));
  t.Merge(s2);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));
}

TEST(Table, SimpleDeferredMerge) {
  Table<int, int> t("t");
  std::set<std::tuple<int, int>> empty = {};
  std::set<std::tuple<int, int>> s1 = {{1, 1}, {2, 2}};
  std::set<std::tuple<int, int>> s2 = {{2, 2}, {3, 3}};
  std::set<std::tuple<int, int>> s3 = {{1, 1}, {2, 2}, {3, 3}};

  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.DeferredMerge(s1);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.DeferredMerge(s2);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.Tick();
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));
}

TEST(Table, SimpleDeferredDelete) {
  Table<int, int> t("t");
  std::set<std::tuple<int, int>> empty = {};
  std::set<std::tuple<int, int>> s1 = {{1, 1}, {2, 2}};
  std::set<std::tuple<int, int>> s2 = {{2, 2}, {3, 3}};
  std::set<std::tuple<int, int>> s3 = {{1, 1}, {2, 2}, {3, 3}};

  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.Merge(s1);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s1));
  t.Merge(s2);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));

  t.DeferredDelete(s1);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));
  t.DeferredDelete(s2);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));
  t.Tick();
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
}

TEST(Table, DeferredMergeAndDelete) {
  Table<int, int> t("t");
  std::set<std::tuple<int, int>> empty = {};
  std::set<std::tuple<int, int>> s1 = {{1, 1}, {2, 2}};
  std::set<std::tuple<int, int>> s2 = {{2, 2}, {3, 3}};
  std::set<std::tuple<int, int>> s3 = {{1, 1}};

  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.DeferredMerge(s1);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.DeferredDelete(s2);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.Tick();
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));
}

TEST(Table, TickDoesntClearTable) {
  Table<int, int> t("t");
  std::set<std::tuple<int, int>> empty;
  std::set<std::tuple<int, int>> s = {{1, 1}, {2, 2}};

  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.Merge(s);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s));
  t.Tick();
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s));
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
