#include "ra/physical/hash_join.h"

#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/physical/iterable.h"
#include "testing/test_util.h"

namespace pra = fluent::ra::physical;

namespace fluent {

void IntIntJoin(const std::set<std::tuple<int>>& left,
                const std::set<std::tuple<int>>& right,
                const std::set<std::tuple<int, int>>& expected) {
  auto left_iterable = pra::make_iterable(&left);
  auto right_iterable = pra::make_iterable(&right);
  using left_keys = ra::LeftKeys<0>;
  using right_keys = ra::RightKeys<0>;
  using left_column_tuple = std::tuple<int>;
  using left_key_column_tuple = std::tuple<int>;
  auto hash_join = pra::make_hash_join<left_keys, right_keys, left_column_tuple,
                                       left_key_column_tuple>(
      std::move(left_iterable), std::move(right_iterable));
  ExpectRngsUnorderedEqual(hash_join.ToRange(), expected);
}

TEST(HashJoin, EmptyEmptyJoin) {
  std::set<std::tuple<int>> left;
  std::set<std::tuple<int>> right;
  std::set<std::tuple<int, int>> expected;
  IntIntJoin(left, right, expected);
}

TEST(HashJoin, RightEmptyJoin) {
  std::set<std::tuple<int>> left = {{1}, {2}, {3}};
  std::set<std::tuple<int>> right;
  std::set<std::tuple<int, int>> expected;
  IntIntJoin(left, right, expected);
}

TEST(HashJoin, LeftmptyJoin) {
  std::set<std::tuple<int>> left;
  std::set<std::tuple<int>> right = {{1}, {2}, {3}};
  std::set<std::tuple<int, int>> expected;
  IntIntJoin(left, right, expected);
}

TEST(HashJoin, NonEmptyJoin) {
  std::set<std::tuple<int, float>> left = {{1, 1.0}, {1, 2.0}, {2, 3.0},
                                           {3, 4.0}, {3, 5.0}, {4, 6.0}};
  std::set<std::tuple<int, char>> right = {{1, 'a'}, {1, 'b'}, {2, 'c'},
                                           {2, 'd'}, {3, 'e'}, {5, 'f'}};
  auto left_iterable = pra::make_iterable(&left);
  auto right_iterable = pra::make_iterable(&right);
  using left_keys = ra::LeftKeys<0>;
  using right_keys = ra::RightKeys<0>;
  using left_column_tuple = std::tuple<int, float>;
  using left_key_column_tuple = std::tuple<int>;
  auto hash_join = pra::make_hash_join<left_keys, right_keys, left_column_tuple,
                                       left_key_column_tuple>(
      std::move(left_iterable), std::move(right_iterable));
  std::set<std::tuple<int, float, int, char>> expected = {
      {1, 1.0, 1, 'a'}, {1, 2.0, 1, 'a'}, {1, 1.0, 1, 'b'}, {1, 2.0, 1, 'b'},
      {2, 3.0, 2, 'c'}, {2, 3.0, 2, 'd'}, {3, 4.0, 3, 'e'}, {3, 5.0, 3, 'e'}};
  ExpectRngsUnorderedEqual(hash_join.ToRange(), expected);
}

TEST(HashJoin, MultiColumnJoin) {
  std::set<std::tuple<int, float>> left = {{1, 1.0}, {2, 2.0}, {3, 3.0}};
  std::set<std::tuple<float, int>> right = {{2.0, 2}, {3.0, 3}, {4.0, 4}};
  auto left_iterable = pra::make_iterable(&left);
  auto right_iterable = pra::make_iterable(&right);
  using left_keys = ra::LeftKeys<0, 1>;
  using right_keys = ra::RightKeys<1, 0>;
  using left_column_tuple = std::tuple<int, float>;
  using left_key_column_tuple = std::tuple<int, float>;
  auto hash_join = pra::make_hash_join<left_keys, right_keys, left_column_tuple,
                                       left_key_column_tuple>(
      std::move(left_iterable), std::move(right_iterable));
  std::set<std::tuple<int, float, int, char>> expected = {{2, 2.0, 2.0, 2},
                                                          {3, 3.0, 3.0, 3}};
  ExpectRngsUnorderedEqual(hash_join.ToRange(), expected);
}

TEST(HashJoin, RepeatedColumnJoin) {
  std::set<std::tuple<int>> left = {{1}, {2}, {3}};
  std::set<std::tuple<int, int>> right = {{1, 1}, {1, 2}, {2, 2},
                                          {2, 1}, {3, 3}, {3, 4}};
  auto left_iterable = pra::make_iterable(&left);
  auto right_iterable = pra::make_iterable(&right);
  using left_keys = ra::LeftKeys<0, 0>;
  using right_keys = ra::RightKeys<0, 1>;
  using left_column_tuple = std::tuple<int>;
  using left_key_column_tuple = std::tuple<int, int>;
  auto hash_join = pra::make_hash_join<left_keys, right_keys, left_column_tuple,
                                       left_key_column_tuple>(
      std::move(left_iterable), std::move(right_iterable));
  std::set<std::tuple<int, int, int>> expected = {
      {1, 1, 1}, {2, 2, 2}, {3, 3, 3}};
  ExpectRngsUnorderedEqual(hash_join.ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
