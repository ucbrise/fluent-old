#include "ra/hash_join.h"

#include <set>
#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/hash_util.h"
#include "common/type_list.h"
#include "ra/iterable.h"
#include "testing/test_util.h"

namespace fluent {

TEST(HashJoin, EmptyEmptyJoin) {
  std::set<std::tuple<int>> left = {};
  std::set<std::tuple<int>> right = {};
  auto joined = ra::make_hash_join<ra::LeftKeys<0>, ra::RightKeys<0>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));
  std::set<ra::LineagedTuple<int, int>> expected = {};

  static_assert(
      std::is_same<decltype(joined)::column_types, TypeList<int, int>>::value,
      "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
  EXPECT_EQ(joined.ToDebugString(), "HashJoin<Left<0>, Right<0>>(left, right)");
}

TEST(HashJoin, RightEmptyJoin) {
  std::set<std::tuple<int>> left = {{1}, {2}, {3}};
  std::set<std::tuple<int>> right = {};
  auto joined = ra::make_hash_join<ra::LeftKeys<0>, ra::RightKeys<0>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));
  std::set<ra::LineagedTuple<int, int>> expected = {};

  static_assert(
      std::is_same<decltype(joined)::column_types, TypeList<int, int>>::value,
      "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
  EXPECT_EQ(joined.ToDebugString(), "HashJoin<Left<0>, Right<0>>(left, right)");
}

TEST(HashJoin, LeftEmptyJoin) {
  std::set<std::tuple<int>> left = {};
  std::set<std::tuple<int>> right = {{1}, {2}, {3}};
  auto joined = ra::make_hash_join<ra::LeftKeys<0>, ra::RightKeys<0>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));
  std::set<ra::LineagedTuple<int, int>> expected = {};

  static_assert(
      std::is_same<decltype(joined)::column_types, TypeList<int, int>>::value,
      "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
  EXPECT_EQ(joined.ToDebugString(), "HashJoin<Left<0>, Right<0>>(left, right)");
}

TEST(HashJoin, NonEmptyJoin) {
  Hash<std::tuple<int, float>> hash_left;
  std::tuple<int, float> left0 = {1, 1.0};
  std::tuple<int, float> left1 = {1, 2.0};
  std::tuple<int, float> left2 = {2, 3.0};
  std::tuple<int, float> left3 = {3, 4.0};
  std::tuple<int, float> left4 = {3, 5.0};
  std::tuple<int, float> left5 = {4, 6.0};
  std::set<std::tuple<int, float>> left = {left0, left1, left2,
                                           left3, left4, left5};

  Hash<std::tuple<int, char>> hash_right;
  std::tuple<int, char> right0 = {1, 'a'};
  std::tuple<int, char> right1 = {1, 'b'};
  std::tuple<int, char> right2 = {2, 'c'};
  std::tuple<int, char> right3 = {2, 'd'};
  std::tuple<int, char> right4 = {3, 'e'};
  std::tuple<int, char> right5 = {5, 'f'};
  std::set<std::tuple<int, char>> right = {right0, right1, right2,
                                           right3, right4, right5};

  auto joined = ra::make_hash_join<ra::LeftKeys<0>, ra::RightKeys<0>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));

  using JoinedTuple = std::tuple<int, float, int, char>;
  std::set<ra::LineagedTuple<int, float, int, char>> expected = {
      ra::make_lineaged_tuple(
          {{"left", hash_left(left0)}, {"right", hash_right(right0)}},
          JoinedTuple{1, 1.0, 1, 'a'}),
      ra::make_lineaged_tuple(
          {{"left", hash_left(left1)}, {"right", hash_right(right0)}},
          JoinedTuple{1, 2.0, 1, 'a'}),
      ra::make_lineaged_tuple(
          {{"left", hash_left(left0)}, {"right", hash_right(right1)}},
          JoinedTuple{1, 1.0, 1, 'b'}),
      ra::make_lineaged_tuple(
          {{"left", hash_left(left1)}, {"right", hash_right(right1)}},
          JoinedTuple{1, 2.0, 1, 'b'}),
      ra::make_lineaged_tuple(
          {{"left", hash_left(left2)}, {"right", hash_right(right2)}},
          JoinedTuple{2, 3.0, 2, 'c'}),
      ra::make_lineaged_tuple(
          {{"left", hash_left(left2)}, {"right", hash_right(right3)}},
          JoinedTuple{2, 3.0, 2, 'd'}),
      ra::make_lineaged_tuple(
          {{"left", hash_left(left3)}, {"right", hash_right(right4)}},
          JoinedTuple{3, 4.0, 3, 'e'}),
      ra::make_lineaged_tuple(
          {{"left", hash_left(left4)}, {"right", hash_right(right4)}},
          JoinedTuple{3, 5.0, 3, 'e'}),
  };

  static_assert(std::is_same<decltype(joined)::column_types,
                             TypeList<int, float, int, char>>::value,
                "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
  EXPECT_EQ(joined.ToDebugString(), "HashJoin<Left<0>, Right<0>>(left, right)");
}

TEST(HashJoin, MultiColumnJoin) {
  Hash<std::tuple<int, float>> hash_left;
  std::tuple<int, float> left1 = {1, 1.0};
  std::tuple<int, float> left2 = {2, 2.0};
  std::tuple<int, float> left3 = {3, 3.0};
  std::set<std::tuple<int, float>> left = {left1, left2, left3};

  Hash<std::tuple<float, int>> hash_right;
  std::tuple<float, int> right2 = {2, 2.0};
  std::tuple<float, int> right3 = {3, 3.0};
  std::tuple<float, int> right4 = {4, 4.0};
  std::set<std::tuple<float, int>> right = {right2, right3, right4};

  auto joined = ra::make_hash_join<ra::LeftKeys<0, 1>, ra::RightKeys<1, 0>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));

  std::set<ra::LineagedTuple<int, float, float, int>> expected = {
      ra::make_lineaged_tuple(
          {{"left", hash_left(left2)}, {"right", hash_right(right2)}},
          std::tuple<int, float, float, int>{2, 2.0, 2.0, 2}),
      ra::make_lineaged_tuple(
          {{"left", hash_left(left3)}, {"right", hash_right(right3)}},
          std::tuple<int, float, float, int>{3, 3.0, 3.0, 3}),
  };

  static_assert(std::is_same<decltype(joined)::column_types,
                             TypeList<int, float, float, int>>::value,
                "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
  EXPECT_EQ(joined.ToDebugString(),
            "HashJoin<Left<0, 1>, Right<1, 0>>(left, right)");
}

TEST(HashJoin, RepeatedColumnJoin) {
  Hash<std::tuple<int>> hash_left;
  std::tuple<int> left1 = {1};
  std::tuple<int> left2 = {2};
  std::tuple<int> left3 = {3};
  std::set<std::tuple<int>> left = {left1, left2, left3};

  Hash<std::tuple<int, int>> hash_right;
  std::tuple<int, int> right0 = {1, 1};
  std::tuple<int, int> right1 = {1, 2};
  std::tuple<int, int> right2 = {2, 2};
  std::tuple<int, int> right3 = {2, 1};
  std::tuple<int, int> right4 = {3, 3};
  std::tuple<int, int> right5 = {3, 4};
  std::set<std::tuple<int, int>> right = {right0, right1, right2,
                                          right3, right4, right5};

  auto joined = ra::make_hash_join<ra::LeftKeys<0, 0>, ra::RightKeys<0, 1>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));

  std::set<ra::LineagedTuple<int, int, int>> expected = {
      ra::make_lineaged_tuple(
          {{"left", hash_left(left1)}, {"right", hash_right(right0)}},
          std::tuple<int, int, int>{1, 1, 1}),
      ra::make_lineaged_tuple(
          {{"left", hash_left(left2)}, {"right", hash_right(right2)}},
          std::tuple<int, int, int>{2, 2, 2}),
      ra::make_lineaged_tuple(
          {{"left", hash_left(left3)}, {"right", hash_right(right4)}},
          std::tuple<int, int, int>{3, 3, 3}),
  };

  static_assert(std::is_same<decltype(joined)::column_types,
                             TypeList<int, int, int>>::value,
                "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
  EXPECT_EQ(joined.ToDebugString(),
            "HashJoin<Left<0, 0>, Right<0, 1>>(left, right)");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
