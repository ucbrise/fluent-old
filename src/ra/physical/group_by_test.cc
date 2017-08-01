#include "ra/physical/group_by.h"

#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/sizet_list.h"
#include "common/type_list.h"
#include "ra/aggregates.h"
#include "ra/physical/iterable.h"
#include "testing/test_util.h"

namespace pra = fluent::ra::physical;

namespace fluent {

TEST(GroupBy, SimpleCount) {
  std::set<std::tuple<int, int>> xs = {{0, 0}, {0, 1}, {1, 2},
                                       {1, 3}, {1, 4}, {2, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls = std::tuple<
      ra::agg::CountImpl<common::SizetList<1>, common::TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, std::size_t>> expected = {{0, 2}, {1, 3}, {2, 1}};
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, VariadicCount) {
  std::set<std::tuple<int, int, int, int>> xs = {{0, 0, 0, 0}, {0, 1, 1, 1},
                                                 {1, 2, 2, 2}, {1, 3, 3, 3},
                                                 {1, 4, 4, 4}, {2, 5, 5, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls = std::tuple<
      ra::agg::CountImpl<common::SizetList<1, 2, 3, 1, 2, 3>,
                         common::TypeList<int, int, int, int, int, int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, std::size_t>> expected = {{0, 2}, {1, 3}, {2, 1}};
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, EmptyCount) {
  std::set<std::tuple<int, int, int, int>> xs = {{0, 0, 0, 0}, {0, 1, 1, 1},
                                                 {1, 2, 2, 2}, {1, 3, 3, 3},
                                                 {1, 4, 4, 4}, {2, 5, 5, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls =
      std::tuple<ra::agg::CountImpl<common::SizetList<>, common::TypeList<>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, std::size_t>> expected = {{0, 2}, {1, 3}, {2, 1}};
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, SimpleSum) {
  std::set<std::tuple<int, int>> xs = {{0, 0}, {0, 1}, {1, 2},
                                       {1, 3}, {1, 4}, {2, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls =
      std::tuple<ra::agg::SumImpl<common::SizetList<1>, common::TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, int>> expected = {{0, 1}, {1, 9}, {2, 5}};
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, VariadicSum) {
  std::set<std::tuple<int, int, int, int>> xs = {{0, 0, 0, 0}, {0, 1, 1, 1},
                                                 {1, 2, 2, 2}, {1, 3, 3, 3},
                                                 {1, 4, 4, 4}, {2, 5, 5, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls = std::tuple<
      ra::agg::SumImpl<common::SizetList<1, 2, 3, 1, 2, 3>,
                       common::TypeList<int, int, int, int, int, int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, int>> expected = {{0, 6}, {1, 54}, {2, 30}};
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, SimpleAvg) {
  std::set<std::tuple<int, int>> xs = {{0, 0}, {0, 1}, {1, 2},
                                       {1, 3}, {1, 4}, {2, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls =
      std::tuple<ra::agg::AvgImpl<common::SizetList<1>, common::TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, int>> expected = {{0, 0.5}, {1, 3.0}, {2, 5.0}};
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, VariadicAvg) {
  std::set<std::tuple<int, int, int>> xs = {{0, 0, 2}, {0, 1, 3}, {1, 2, 5},
                                            {1, 3, 6}, {1, 4, 7}, {2, 5, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls = std::tuple<
      ra::agg::AvgImpl<common::SizetList<1, 2>, common::TypeList<int, int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, int>> expected = {{0, 1.5}, {1, 4.5}, {2, 5.0}};
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, SimpleUnion) {
  std::set<std::tuple<int, std::set<int>>> xs = {
      std::make_tuple(0, std::set<int>({0})),
      std::make_tuple(0, std::set<int>({1})),
      std::make_tuple(1, std::set<int>({2})),
      std::make_tuple(1, std::set<int>({3})),
      std::make_tuple(1, std::set<int>({4})),
      std::make_tuple(2, std::set<int>({5}))};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls =
      std::tuple<ra::agg::UnionImpl<common::SizetList<1>,
                                    common::TypeList<std::set<int>>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, std::set<int>>> expected = {
      std::make_tuple(0, std::set<int>({0, 1})),
      std::make_tuple(1, std::set<int>({2, 3, 4})),
      std::make_tuple(2, std::set<int>({5}))};
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, VariadicUnion) {
  std::set<std::tuple<int, std::set<int>, std::set<int>>> xs = {
      std::make_tuple(0, std::set<int>({0}), std::set<int>({2})),
      std::make_tuple(0, std::set<int>({1}), std::set<int>({3})),
      std::make_tuple(1, std::set<int>({2}), std::set<int>({5})),
      std::make_tuple(1, std::set<int>({3}), std::set<int>({6})),
      std::make_tuple(1, std::set<int>({4}), std::set<int>({7})),
      std::make_tuple(2, std::set<int>({5}), std::set<int>({5}))};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls = std::tuple<ra::agg::UnionImpl<
      common::SizetList<1, 2>, common::TypeList<std::set<int>, std::set<int>>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, std::set<int>>> expected = {
      std::make_tuple(0, std::set<int>({0, 1, 2, 3})),
      std::make_tuple(1, std::set<int>({2, 3, 4, 5, 6, 7})),
      std::make_tuple(2, std::set<int>({5}))};
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, SimpleSumCountAvg) {
  std::set<std::tuple<int, int, int, int>> xs = {
      {1, 2, 9, 1}, {1, 3, 8, 2}, {1, 1, 0, 3}, {2, 1, 5, 3},
      {2, 2, 9, 4}, {2, 8, 3, 5}, {3, 3, 9, 4}, {3, 2, 3, 5},
      {3, 1, 1, 6}, {3, 0, 0, 7}, {3, 0, 0, 8}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls = std::tuple<
      ra::agg::SumImpl<common::SizetList<1>, common::TypeList<int>>,
      ra::agg::CountImpl<common::SizetList<2>, common::TypeList<int>>,
      ra::agg::AvgImpl<common::SizetList<3>, common::TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, int, std::size_t, double>> expected = {
      {1, 6, 3, 2.0}, {2, 11, 3, 4.0}, {3, 6, 5, 6.0}};
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, NoKeys) {
  std::set<std::tuple<int>> xs = {{1}, {2}, {3}, {4}, {5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<>;
  using key_tuple = std::tuple<>;
  using agg_impls = std::tuple<
      ra::agg::CountImpl<common::SizetList<0>, common::TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<std::size_t>> expected = {{5}};
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, EmptyChildNoKeys) {
  std::set<std::tuple<int>> xs;
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<>;
  using key_tuple = std::tuple<>;
  using agg_impls = std::tuple<
      ra::agg::CountImpl<common::SizetList<0>, common::TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<std::size_t>> expected;
  testing::ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
