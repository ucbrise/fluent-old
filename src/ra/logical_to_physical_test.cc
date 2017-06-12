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
  std::set<LocalTupleId> tups1 = {LocalTupleId{"t", std::size_t(1), 42}};
  std::set<LocalTupleId> tups2 = {LocalTupleId{"t", std::size_t(2), 42}};
  std::set<LocalTupleId> tups3 = {LocalTupleId{"t", std::size_t(3), 42}};
  std::set<Lineaged<std::tuple<int>>> expected = {
      std::make_tuple(std::make_tuple(1), tups1),
      std::make_tuple(std::make_tuple(2), tups2),
      std::make_tuple(std::make_tuple(3), tups3)};
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
  LocalTupleId tup1 = {"t", std::size_t(1), 42};
  LocalTupleId tup2 = {"t", std::size_t(2), 42};
  LocalTupleId tup3 = {"t", std::size_t(3), 42};
  LocalTupleId tup4 = {"t", std::size_t(3), 43};
  LocalTupleId tup5 = {"t", std::size_t(3), 44};
  std::set<LocalTupleId> tups1 = {tup1};
  std::set<LocalTupleId> tups2 = {tup2};
  std::set<LocalTupleId> tups3 = {tup3};
  std::set<LocalTupleId> tups4 = {tup4};
  std::set<LocalTupleId> tups5 = {tup5};
  std::set<Lineaged<std::tuple<std::tuple<int>, LocalTupleId>>> expected = {
      std::make_tuple(std::make_tuple(std::make_tuple(1), tup1), tups1),
      std::make_tuple(std::make_tuple(std::make_tuple(2), tup2), tups2),
      std::make_tuple(std::make_tuple(std::make_tuple(3), tup3), tups3),
      std::make_tuple(std::make_tuple(std::make_tuple(3), tup4), tups4),
      std::make_tuple(std::make_tuple(std::make_tuple(3), tup5), tups5)};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

TEST(LogicalToPhysical, Iterable) {
  std::set<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto logical = lra::make_iterable(&xs);
  auto physical = ra::LogicalToPhysical(logical);
  std::set<LocalTupleId> empty_tups = {};
  std::set<Lineaged<std::tuple<int>>> expected = {
      std::make_tuple(std::make_tuple(1), empty_tups), 
      std::make_tuple(std::make_tuple(2), empty_tups), 
      std::make_tuple(std::make_tuple(3), empty_tups)};
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
  std::set<LocalTupleId> tups1 = {LocalTupleId{"t", std::size_t(1), 42}};
  std::set<LocalTupleId> tups2 = {LocalTupleId{"t", std::size_t(2), 42}};
  std::set<LocalTupleId> tups3 = {LocalTupleId{"t", std::size_t(3), 42}};
  std::set<Lineaged<std::tuple<int, int>>> expected = {
      std::make_tuple(std::make_tuple(1, 1), tups1),
      std::make_tuple(std::make_tuple(2, 2), tups2),
      std::make_tuple(std::make_tuple(3, 3), tups3)};
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
  std::set<LocalTupleId> tups1 = {LocalTupleId{"t", std::size_t(1), 42}};
  std::set<LocalTupleId> tups2 = {LocalTupleId{"t", std::size_t(2), 42}};
  std::set<LocalTupleId> tups3 = {LocalTupleId{"t", std::size_t(3), 42}};
  std::set<Lineaged<std::tuple<int>>> expected = {
      std::make_tuple(std::make_tuple(1), tups1),
      std::make_tuple(std::make_tuple(2), tups2),
      std::make_tuple(std::make_tuple(3), tups3)};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

TEST(LogicalToPhysical, Project) {
  Table<int, int> t("t", {{"x", "y"}});
  t.Merge({1, 10}, 1, 42);
  t.Merge({2, 20}, 2, 42);
  t.Merge({3, 30}, 3, 42);
  auto logical = lra::make_collection(&t) | lra::project<0>();
  auto physical = ra::LogicalToPhysical(logical);
  std::set<LocalTupleId> tups1 = {LocalTupleId{"t", std::size_t(1), 42}};
  std::set<LocalTupleId> tups2 = {LocalTupleId{"t", std::size_t(2), 42}};
  std::set<LocalTupleId> tups3 = {LocalTupleId{"t", std::size_t(3), 42}};
  std::set<Lineaged<std::tuple<int>>> expected = {
      std::make_tuple(std::make_tuple(1), tups1),
      std::make_tuple(std::make_tuple(2), tups2),
      std::make_tuple(std::make_tuple(3), tups3)};
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
  LocalTupleId tup1 = {"t", std::size_t(1), 42};
  LocalTupleId tup2 = {"t", std::size_t(2), 42};
  LocalTupleId tup3 = {"r", std::size_t(10), 9001};
  LocalTupleId tup4 = {"r", std::size_t(20), 9001};
  std::set<LocalTupleId> tups1 = {tup1, tup3};
  std::set<LocalTupleId> tups2 = {tup1, tup4};
  std::set<LocalTupleId> tups3 = {tup2, tup3};
  std::set<LocalTupleId> tups4 = {tup2, tup4};
  std::set<Lineaged<std::tuple<int, int>>> expected = {
      std::make_tuple(std::make_tuple(1, 1), tups1),
      std::make_tuple(std::make_tuple(1, 2), tups2),
      std::make_tuple(std::make_tuple(2, 1), tups3),
      std::make_tuple(std::make_tuple(2, 2), tups4)};
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
  LocalTupleId tup1 = {"t", std::size_t(1), 42};
  LocalTupleId tup2 = {"t", std::size_t(2), 42};
  LocalTupleId tup3 = {"t", std::size_t(3), 42};
  LocalTupleId tup4 = {"r", std::size_t(10), 9001};
  LocalTupleId tup5 = {"r", std::size_t(20), 9001};
  LocalTupleId tup6 = {"r", std::size_t(30), 9001};
  std::set<LocalTupleId> tups1 = {tup1, tup4};
  std::set<LocalTupleId> tups2 = {tup2, tup5};
  std::set<LocalTupleId> tups3 = {tup3, tup6};
  std::set<Lineaged<std::tuple<int, int>>> expected = {
      std::make_tuple(std::make_tuple(1, 1), tups1),
      std::make_tuple(std::make_tuple(2, 2), tups2),
      std::make_tuple(std::make_tuple(3, 3), tups3)};
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
  LocalTupleId tup1 = {"t", std::size_t(100), 42};
  LocalTupleId tup2 = {"t", std::size_t(200), 42};
  LocalTupleId tup3 = {"t", std::size_t(300), 42};
  LocalTupleId tup4 = {"t", std::size_t(400), 42};
  LocalTupleId tup5 = {"t", std::size_t(500), 42};
  LocalTupleId tup6 = {"t", std::size_t(600), 42};
  std::set<LocalTupleId> tups1 = {tup1, tup2};
  std::set<LocalTupleId> tups2 = {tup3, tup4, tup5};
  std::set<LocalTupleId> tups3 = {tup6};
  std::set<Lineaged<std::tuple<int, int>>> expected = {
      std::make_tuple(std::make_tuple(1, 30), tups1),
      std::make_tuple(std::make_tuple(2, 120), tups2),
      std::make_tuple(std::make_tuple(3, 60), tups3)};
  ExpectRngsUnorderedEqual(physical.ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
