#include "ra/logical_to_physical.h"

#include <set>
#include <tuple>
#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "collections/table.h"
#include "ra/logical/all.h"
#include "ra/physical/all.h"
#include "testing/test_util.h"

namespace lra = fluent::ra::logical;
namespace pra = fluent::ra::physical;

namespace fluent {
namespace {

template <typename T>
using Lineaged = std::tuple<T, std::set<LocalTupleId>>;

}  // namespace

TEST(LogicalToPhysical, Collection) {
  Table<int> t("t", {{"x"}});
  t.Merge({1}, 1, 42);
  t.Merge({2}, 2, 42);
  t.Merge({3}, 3, 42);
  auto logical = lra::make_collection(&t);
  auto physical = ra::LogicalToPhysical(logical);
  std::set<Lineaged<std::tuple<int>>> expected = {
      {{1}, {{"t", 1, 42}}}, {{2}, {{"t", 2, 42}}}, {{3}, {{"t", 3, 42}}}};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

TEST(LogicalToPhysical, MetaCollection) {
  Table<int> t("t", {{"x"}});
  t.Merge({1}, 1, 42);
  t.Merge({2}, 2, 42);
  t.Merge({3}, 3, 42);
  t.Merge({3}, 3, 43);
  t.Merge({3}, 3, 44);
  auto logical = lra::make_meta_collection(&t);
  auto physical = ra::LogicalToPhysical(logical);
  std::set<Lineaged<std::tuple<std::tuple<int>, LocalTupleId>>> expected = {
      {{{1}, {"t", 1, 42}}, {{"t", 1, 42}}},
      {{{2}, {"t", 2, 42}}, {{"t", 2, 42}}},
      {{{3}, {"t", 3, 42}}, {{"t", 3, 42}}},
      {{{3}, {"t", 3, 43}}, {{"t", 3, 43}}},
      {{{3}, {"t", 3, 44}}, {{"t", 3, 44}}}};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

TEST(LogicalToPhysical, Iterable) {
  std::set<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto logical = lra::make_iterable(&xs);
  auto physical = ra::LogicalToPhysical(logical);
  std::set<Lineaged<std::tuple<int>>> expected = {
      {{1}, {}}, {{2}, {}}, {{3}, {}}};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

TEST(LogicalToPhysical, Map) {
  Table<int> t("t", {{"x"}});
  t.Merge({1}, 1, 42);
  t.Merge({2}, 2, 42);
  t.Merge({3}, 3, 42);
  auto logical = lra::make_collection(&t) |
                 lra::map([](const auto& t) { return std::tuple_cat(t, t); });
  auto physical = ra::LogicalToPhysical(logical);
  std::set<Lineaged<std::tuple<int, int>>> expected = {
      {{1, 1}, {{"t", 1, 42}}},
      {{2, 2}, {{"t", 2, 42}}},
      {{3, 3}, {{"t", 3, 42}}}};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

TEST(LogicalToPhysical, Filter) {
  Table<int> t("t", {{"x"}});
  t.Merge({1}, 1, 42);
  t.Merge({2}, 2, 42);
  t.Merge({3}, 3, 42);
  auto logical =
      lra::make_collection(&t) | lra::filter([](const auto&) { return true; });
  auto physical = ra::LogicalToPhysical(logical);
  std::set<Lineaged<std::tuple<int>>> expected = {
      {{1}, {{"t", 1, 42}}}, {{2}, {{"t", 2, 42}}}, {{3}, {{"t", 3, 42}}}};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

TEST(LogicalToPhysical, Project) {
  Table<int, int> t("t", {{"x", "y"}});
  t.Merge({1, 10}, 1, 42);
  t.Merge({2, 20}, 2, 42);
  t.Merge({3, 30}, 3, 42);
  auto logical = lra::make_collection(&t) | lra::project<0>();
  auto physical = ra::LogicalToPhysical(logical);
  std::set<Lineaged<std::tuple<int>>> expected = {
      {{1}, {{"t", 1, 42}}}, {{2}, {{"t", 2, 42}}}, {{3}, {{"t", 3, 42}}}};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

TEST(LogicalToPhysical, Cross) {
  Table<int> t("t", {{"x"}});
  t.Merge({1}, 1, 42);
  t.Merge({2}, 2, 42);

  Table<int> r("r", {{"y"}});
  r.Merge({1}, 10, 9001);
  r.Merge({2}, 20, 9001);

  auto logical =
      lra::make_cross(lra::make_collection(&t), lra::make_collection(&r));
  auto physical = ra::LogicalToPhysical(logical);
  std::set<Lineaged<std::tuple<int, int>>> expected = {
      {{1, 1}, {{"t", 1, 42}, {"r", 10, 9001}}},
      {{1, 2}, {{"t", 1, 42}, {"r", 20, 9001}}},
      {{2, 1}, {{"t", 2, 42}, {"r", 10, 9001}}},
      {{2, 2}, {{"t", 2, 42}, {"r", 20, 9001}}}};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

TEST(LogicalToPhysical, HashJoin) {
  Table<int> t("t", {{"x"}});
  t.Merge({1}, 1, 42);
  t.Merge({2}, 2, 42);
  t.Merge({3}, 3, 42);

  Table<int> r("r", {{"y"}});
  r.Merge({1}, 10, 9001);
  r.Merge({2}, 20, 9001);
  r.Merge({3}, 30, 9001);

  auto logical = lra::make_hash_join<ra::LeftKeys<0>, ra::RightKeys<0>>(
      lra::make_collection(&t), lra::make_collection(&r));
  auto physical = ra::LogicalToPhysical(logical);
  std::set<Lineaged<std::tuple<int, int>>> expected = {
      {{1, 1}, {{"t", 1, 42}, {"r", 10, 9001}}},
      {{2, 2}, {{"t", 2, 42}, {"r", 20, 9001}}},
      {{3, 3}, {{"t", 3, 42}, {"r", 30, 9001}}}};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

TEST(LogicalToPhysical, GroupBy) {
  Table<int, int> t("t", {{"x", "y"}});
  t.Merge({1, 10}, 100, 42);
  t.Merge({1, 20}, 200, 42);
  t.Merge({2, 30}, 300, 42);
  t.Merge({2, 40}, 400, 42);
  t.Merge({2, 50}, 500, 42);
  t.Merge({3, 60}, 600, 42);

  auto logical =
      lra::make_collection(&t) | lra::group_by<ra::Keys<0>, ra::agg::Sum<1>>();
  auto physical = ra::LogicalToPhysical(logical);
  std::set<Lineaged<std::tuple<int, int>>> expected = {
      {{1, 30}, {{"t", 100, 42}, {"t", 200, 42}}},
      {{2, 120}, {{"t", 300, 42}, {"t", 400, 42}, {"t", 500, 42}}},
      {{3, 60}, {{"t", 600, 42}}}};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
