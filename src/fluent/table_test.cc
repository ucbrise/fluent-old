#include "fluent/table.h"

#include <set>
#include <tuple>
#include <utility>
#include <vector>

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
  std::set<std::tuple<int, int>> s1 = {{1, 1}, {2, 2}};
  std::set<std::tuple<int, int>> s2 = {{2, 2}, {3, 3}};
  std::set<std::tuple<int, int>> s3 = {{1, 1}, {2, 2}, {3, 3}};

  t.Merge(ra::make_iterable(&s1));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s1));
  t.Merge(ra::make_iterable(&s2));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));
}

TEST(Table, SimpleSetMerge) {
  Table<int, int> t("t");
  std::set<std::tuple<int, int>> s1 = {{1, 1}, {2, 2}};
  std::set<std::tuple<int, int>> s2 = {{2, 2}, {3, 3}};
  std::set<std::tuple<int, int>> s3 = {{1, 1}, {2, 2}, {3, 3}};

  t.Merge(s1);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s1));
  t.Merge(s2);
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));
}

TEST(Table, SimpleMergeWithVectors) {
  Table<int, int> t("t");
  std::vector<std::tuple<int, int>> s1 = {{1, 1}, {2, 2}};
  std::vector<std::tuple<int, int>> s2 = {{2, 2}, {3, 3}};
  std::vector<std::tuple<int, int>> s3 = {{1, 1}, {2, 2}, {3, 3}};

  t.Merge(ra::make_iterable(&s1));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s1));
  t.Merge(ra::make_iterable(&s2));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));
}

TEST(Table, SelfMerge) {
  Table<int, int> t("t");
  std::set<std::tuple<int, int>> s = {{1, 1}, {2, 2}};

  t.Merge(ra::make_iterable(&s));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s));
  t.Merge(ra::make_iterable(&t.Get()));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s));
}

TEST(Table, CompositeQueryMerge) {
  using Tuple = std::tuple<int, int, int>;
  using TupleSet = std::set<Tuple>;

  std::set<Tuple> ts;
  for (int i = 0; i < 10; ++i) {
    ts.insert(Tuple(i, i, i));
  }

  Table<int, int, int> t("t");
  t.Merge(ra::make_iterable(&ts));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(ts));

  // clang-format off
  t.Merge(t.Iterable()
    | ra::filter([](const auto& t) {
        return fst(t) == snd(t) && snd(t) == thd(t);
      })
    | ra::map([](const auto& t) {
        return Tuple(fst(t) * 2, fst(t) * 2, fst(t) * 2);
      })
    | ra::filter([](const auto& t) {
        return fst(t) + snd(t) + thd(t) < 50;
      }));
  // clang-format on

  TupleSet expected;
  for (int i = 0; i < 10; ++i) {
    expected.insert(Tuple(i, i, i));
    const int doubled = i * 2;
    if (doubled * 3 < 50) {
      expected.insert(Tuple(doubled, doubled, doubled));
    }
  }
  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(expected));
}

TEST(Table, TickDoesntClearTable) {
  Table<int, int> t("t");
  std::set<std::tuple<int, int>> s = {{1, 1}, {2, 2}};
  t.Merge(ra::make_iterable(&s));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s));
  t.Tick();
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s));
}

TEST(Table, SimpleDeferredMerge) {
  Table<int, int> t("t");
  std::set<std::tuple<int, int>> empty = {};
  std::set<std::tuple<int, int>> s1 = {{1, 1}, {2, 2}};
  std::set<std::tuple<int, int>> s2 = {{2, 2}, {3, 3}};
  std::set<std::tuple<int, int>> s3 = {{1, 1}, {2, 2}, {3, 3}};

  t.DeferredMerge(ra::make_iterable(&s1));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.DeferredMerge(ra::make_iterable(&s2));
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

  t.Merge(ra::make_iterable(&s1));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s1));
  t.Merge(ra::make_iterable(&s2));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));

  t.DeferredDelete(ra::make_iterable(&s1));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));
  t.DeferredDelete(ra::make_iterable(&s2));
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

  t.DeferredMerge(ra::make_iterable(&s1));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.DeferredDelete(ra::make_iterable(&s2));
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(empty));
  t.Tick();
  EXPECT_THAT(t.Get(), testing::UnorderedElementsAreArray(s3));
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
