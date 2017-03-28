#include "ra/hash_join.h"

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/type_list.h"
#include "ra/iterable.h"
#include "testing/test_util.h"

namespace fluent {

TEST(HashJoin, EmptyEmptyJoin) {
  std::vector<std::tuple<int>> left = {};
  std::vector<std::tuple<int>> right = {};
  std::vector<std::tuple<int, int>> expected = {};
  auto joined = ra::make_hash_join<ra::LeftKeys<0>, ra::RightKeys<0>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));
  static_assert(
      std::is_same<decltype(joined)::column_types, TypeList<int, int>>::value,
      "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
}

TEST(HashJoin, RightEmptyJoin) {
  std::vector<std::tuple<int>> left = {{1}, {2}, {3}};
  std::vector<std::tuple<int>> right = {};
  std::vector<std::tuple<int, int>> expected = {};
  auto joined = ra::make_hash_join<ra::LeftKeys<0>, ra::RightKeys<0>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));
  static_assert(
      std::is_same<decltype(joined)::column_types, TypeList<int, int>>::value,
      "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
}

TEST(HashJoin, LeftEmptyJoin) {
  std::vector<std::tuple<int>> left = {};
  std::vector<std::tuple<int>> right = {{1}, {2}, {3}};
  std::vector<std::tuple<int, int>> expected = {};
  auto joined = ra::make_hash_join<ra::LeftKeys<0>, ra::RightKeys<0>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));
  static_assert(
      std::is_same<decltype(joined)::column_types, TypeList<int, int>>::value,
      "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
}

TEST(HashJoin, NonEmptyJoin) {
  std::vector<std::tuple<int, float>> left = {{1, 1.0}, {1, 2.0}, {2, 3.0},
                                              {3, 4.0}, {3, 5.0}, {4, 6.0}};
  std::vector<std::tuple<int, char>> right = {{1, 'a'}, {1, 'b'}, {2, 'c'},
                                              {2, 'd'}, {3, 'e'}, {5, 'f'}};
  std::vector<std::tuple<int, float, int, char>> expected = {
      {1, 1.0, 1, 'a'}, {1, 2.0, 1, 'a'}, {1, 1.0, 1, 'b'}, {1, 2.0, 1, 'b'},
      {2, 3.0, 2, 'c'}, {2, 3.0, 2, 'd'}, {3, 4.0, 3, 'e'}, {3, 5.0, 3, 'e'}};

  auto joined = ra::make_hash_join<ra::LeftKeys<0>, ra::RightKeys<0>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));
  static_assert(std::is_same<decltype(joined)::column_types,
                             TypeList<int, float, int, char>>::value,
                "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
}

TEST(HashJoin, MultiColumnJoin) {
  std::vector<std::tuple<int, float>> left = {{1, 1.0}, {2, 2.0}, {3, 3.0}};
  std::vector<std::tuple<float, int>> right = {{2, 2.0}, {3, 3.0}, {4, 4.0}};
  std::vector<std::tuple<int, float, float, int>> expected = {{2, 2.0, 2.0, 2},
                                                              {3, 3.0, 3.0, 3}};

  auto joined = ra::make_hash_join<ra::LeftKeys<0, 1>, ra::RightKeys<1, 0>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));
  static_assert(std::is_same<decltype(joined)::column_types,
                             TypeList<int, float, float, int>>::value,
                "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
}

TEST(HashJoin, RepeatedColumnJoin) {
  std::vector<std::tuple<int>> left = {{1}, {2}, {3}};
  std::vector<std::tuple<int, int>> right = {{1, 1}, {1, 2}, {2, 2},
                                             {2, 1}, {3, 3}, {3, 4}};
  std::vector<std::tuple<int, int, int>> expected = {
      {1, 1, 1}, {2, 2, 2}, {3, 3, 3}};

  auto joined = ra::make_hash_join<ra::LeftKeys<0, 0>, ra::RightKeys<0, 1>>(
      ra::make_iterable("left", &left), ra::make_iterable("right", &right));
  static_assert(std::is_same<decltype(joined)::column_types,
                             TypeList<int, int, int>>::value,
                "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
