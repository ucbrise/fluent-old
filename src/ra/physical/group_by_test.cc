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
  using agg_impls = std::tuple<ra::agg::CountImpl<SizetList<1>, TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, std::size_t>> expected = {{0, 2}, {1, 3}, {2, 1}};
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, VariadicCount) {
  std::set<std::tuple<int, int, int, int>> xs = {{0, 0, 0, 0}, {0, 1, 1, 1},
                                                 {1, 2, 2, 2}, {1, 3, 3, 3},
                                                 {1, 4, 4, 4}, {2, 5, 5, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls =
      std::tuple<ra::agg::CountImpl<SizetList<1, 2, 3, 1, 2, 3>,
                                    TypeList<int, int, int, int, int, int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, std::size_t>> expected = {{0, 2}, {1, 3}, {2, 1}};
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, EmptyCount) {
  std::set<std::tuple<int, int, int, int>> xs = {{0, 0, 0, 0}, {0, 1, 1, 1},
                                                 {1, 2, 2, 2}, {1, 3, 3, 3},
                                                 {1, 4, 4, 4}, {2, 5, 5, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls = std::tuple<ra::agg::CountImpl<SizetList<>, TypeList<>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, std::size_t>> expected = {{0, 2}, {1, 3}, {2, 1}};
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, SimpleSum) {
  std::set<std::tuple<int, int>> xs = {{0, 0}, {0, 1}, {1, 2},
                                       {1, 3}, {1, 4}, {2, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls = std::tuple<ra::agg::SumImpl<SizetList<1>, TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, int>> expected = {{0, 1}, {1, 9}, {2, 5}};
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, VariadicSum) {
  std::set<std::tuple<int, int, int, int>> xs = {{0, 0, 0, 0}, {0, 1, 1, 1},
                                                 {1, 2, 2, 2}, {1, 3, 3, 3},
                                                 {1, 4, 4, 4}, {2, 5, 5, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls =
      std::tuple<ra::agg::SumImpl<SizetList<1, 2, 3, 1, 2, 3>,
                                  TypeList<int, int, int, int, int, int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, int>> expected = {{0, 6}, {1, 54}, {2, 30}};
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, SimpleAvg) {
  std::set<std::tuple<int, int>> xs = {{0, 0}, {0, 1}, {1, 2},
                                       {1, 3}, {1, 4}, {2, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls = std::tuple<ra::agg::AvgImpl<SizetList<1>, TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, int>> expected = {{0, 0.5}, {1, 3.0}, {2, 5.0}};
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, VariadicAvg) {
  std::set<std::tuple<int, int, int>> xs = {{0, 0, 2}, {0, 1, 3}, {1, 2, 5},
                                            {1, 3, 6}, {1, 4, 7}, {2, 5, 5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls =
      std::tuple<ra::agg::AvgImpl<SizetList<1, 2>, TypeList<int, int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, int>> expected = {{0, 1.5}, {1, 4.5}, {2, 5.0}};
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, SimpleUnion) {
  std::set<std::tuple<int, std::set<int>>> xs = {{0, {0}}, {0, {1}}, {1, {2}},
                                                 {1, {3}}, {1, {4}}, {2, {5}}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls =
      std::tuple<ra::agg::UnionImpl<SizetList<1>, TypeList<std::set<int>>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, std::set<int>>> expected = {
      {0, {0, 1}}, {1, {2, 3, 4}}, {2, {5}}};
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, VariadicUnion) {
  std::set<std::tuple<int, std::set<int>, std::set<int>>> xs = {
      {0, {0}, {2}}, {0, {1}, {3}}, {1, {2}, {5}},
      {1, {3}, {6}}, {1, {4}, {7}}, {2, {5}, {5}}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls =
      std::tuple<ra::agg::UnionImpl<SizetList<1, 2>,
                                    TypeList<std::set<int>, std::set<int>>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, std::set<int>>> expected = {
      {0, {0, 1, 2, 3}}, {1, {2, 3, 4, 5, 6, 7}}, {2, {5}}};
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, SimpleSumCountAvg) {
  std::set<std::tuple<int, int, int, int>> xs = {
      {1, 2, 9, 1}, {1, 3, 8, 2}, {1, 1, 0, 3}, {2, 1, 5, 3},
      {2, 2, 9, 4}, {2, 8, 3, 5}, {3, 3, 9, 4}, {3, 2, 3, 5},
      {3, 1, 1, 6}, {3, 0, 0, 7}, {3, 0, 0, 8}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<0>;
  using key_tuple = std::tuple<int>;
  using agg_impls = std::tuple<ra::agg::SumImpl<SizetList<1>, TypeList<int>>,
                               ra::agg::CountImpl<SizetList<2>, TypeList<int>>,
                               ra::agg::AvgImpl<SizetList<3>, TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<int, int, std::size_t, double>> expected = {
      {1, 6, 3, 2.0}, {2, 11, 3, 4.0}, {3, 6, 5, 6.0}};
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, NoKeys) {
  std::set<std::tuple<int>> xs = {{1}, {2}, {3}, {4}, {5}};
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<>;
  using key_tuple = std::tuple<>;
  using agg_impls = std::tuple<ra::agg::CountImpl<SizetList<0>, TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<std::size_t>> expected = {{5}};
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

TEST(GroupBy, EmptyChildNoKeys) {
  std::set<std::tuple<int>> xs;
  auto it = pra::make_iterable(&xs);
  using keys = ra::Keys<>;
  using key_tuple = std::tuple<>;
  using agg_impls = std::tuple<ra::agg::CountImpl<SizetList<0>, TypeList<int>>>;
  auto group_by = pra::make_group_by<keys, key_tuple, agg_impls>(std::move(it));
  std::set<std::tuple<std::size_t>> expected;
  ExpectRngsUnorderedEqual(group_by.ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
