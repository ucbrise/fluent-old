#include "ra/logical/to_debug_string.h"

#include <set>
#include <tuple>
#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "collections/table.h"
#include "ra/logical/all.h"

namespace lra = fluent::ra::logical;

namespace fluent {

TEST(ToDebugString, Collection) {
  collections::Table<int> t("t", {{"x"}});
  const auto collection = lra::make_collection(&t);
  const std::string actual = lra::ToDebugString(collection);
  const std::string expected = "Collection(t)";
  EXPECT_EQ(actual, expected);
}

TEST(ToDebugString, MetaCollection) {
  collections::Table<int> t("t", {{"x"}});
  const auto meta_collection = lra::make_meta_collection(&t);
  const std::string actual = lra::ToDebugString(meta_collection);
  const std::string expected = "MetaCollection(t)";
  EXPECT_EQ(actual, expected);
}

TEST(ToDebugString, Iterable) {
  std::set<std::tuple<int>> xs;
  const auto iter = lra::make_iterable(&xs);
  const std::string actual = lra::ToDebugString(iter);
  const std::string expected = "Iterable";
  EXPECT_EQ(actual, expected);
}

TEST(ToDebugString, Map) {
  std::set<std::tuple<int>> xs;
  const auto f = [](const std::tuple<int>& t) { return t; };
  const auto map = lra::make_iterable(&xs) | lra::map(f);
  const std::string actual = lra::ToDebugString(map);
  const std::string expected = "Map(Iterable)";
  EXPECT_EQ(actual, expected);
}

TEST(ToDebugString, Filter) {
  std::set<std::tuple<int>> xs;
  const auto f = [](const std::tuple<int>&) { return true; };
  const auto filter = lra::make_iterable(&xs) | lra::filter(f);
  const std::string actual = lra::ToDebugString(filter);
  const std::string expected = "Filter(Iterable)";
  EXPECT_EQ(actual, expected);
}

TEST(ToDebugString, Project) {
  std::set<std::tuple<int, bool, char>> xs;
  const auto project = lra::make_iterable(&xs) | lra::project<0, 1, 2>();
  const std::string actual = lra::ToDebugString(project);
  const std::string expected = "Project<0, 1, 2>(Iterable)";
  EXPECT_EQ(actual, expected);
}

TEST(ToDebugString, Cross) {
  std::set<std::tuple<int>> xs;
  std::set<std::tuple<bool>> ys;
  const auto iter_xs = lra::make_iterable(&xs);
  const auto iter_ys = lra::make_iterable(&ys);
  const auto cross = lra::make_cross(iter_xs, iter_ys);
  const std::string actual = lra::ToDebugString(cross);
  const std::string expected = "Cross(Iterable, Iterable)";
  EXPECT_EQ(actual, expected);
}

TEST(ToDebugString, HashJoin) {
  std::set<std::tuple<int, bool>> xs;
  std::set<std::tuple<bool, int>> ys;
  const auto ixs = lra::make_iterable(&xs);
  const auto iys = lra::make_iterable(&ys);
  using left_keys = ra::LeftKeys<0, 1>;
  using right_keys = ra::RightKeys<1, 0>;
  const auto hash_join = lra::make_hash_join<left_keys, right_keys>(ixs, iys);
  const std::string actual = lra::ToDebugString(hash_join);
  const std::string expected =
      "HashJoin<LeftKeys<0, 1>, RightKeys<1, 0>>(Iterable, Iterable)";
  EXPECT_EQ(actual, expected);
}

TEST(ToDebugString, GroupBy) {
  std::set<std::tuple<int, bool>> xs;
  const auto group_by =
      lra::make_iterable(&xs) | lra::group_by<ra::Keys<0, 1>>();
  const std::string actual = lra::ToDebugString(group_by);
  const std::string expected = "GroupBy<Keys<0, 1>, >(Iterable)";
  EXPECT_EQ(actual, expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
