#include "ra/project.h"

#include <set>
#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/type_list.h"
#include "ra/iterable.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Project, SimpleProject) {
  Hash<std::tuple<int, char>> hash;
  std::tuple<int, char> t0 = {1, 'a'};
  std::tuple<int, char> t1 = {2, 'b'};
  std::tuple<int, char> t2 = {3, 'c'};
  std::set<std::tuple<int, char>> xs = {t0, t1, t2};

  {
    auto project = ra::make_iterable("xs", &xs) | ra::project<0>();
    std::set<ra::LineagedTuple<int>> expected = {
        ra::make_lineaged_tuple({{"xs", hash(t0)}}, std::make_tuple(1)),
        ra::make_lineaged_tuple({{"xs", hash(t1)}}, std::make_tuple(2)),
        ra::make_lineaged_tuple({{"xs", hash(t2)}}, std::make_tuple(3)),
    };

    static_assert(
        std::is_same<decltype(project)::column_types, TypeList<int>>::value,
        "");
    ExpectRngsUnorderedEqual(project.ToPhysical().ToRange(), expected);
    EXPECT_EQ(project.ToDebugString(), "Project<0>(xs)");
  }

  {
    auto project = ra::make_iterable("xs", &xs) | ra::project<1>();
    std::set<ra::LineagedTuple<char>> expected = {
        ra::make_lineaged_tuple({{"xs", hash(t0)}}, std::make_tuple('a')),
        ra::make_lineaged_tuple({{"xs", hash(t1)}}, std::make_tuple('b')),
        ra::make_lineaged_tuple({{"xs", hash(t2)}}, std::make_tuple('c')),
    };

    static_assert(
        std::is_same<decltype(project)::column_types, TypeList<char>>::value,
        "");
    ExpectRngsUnorderedEqual(project.ToPhysical().ToRange(), expected);
    EXPECT_EQ(project.ToDebugString(), "Project<1>(xs)");
  }

  {
    auto project = ra::make_iterable("xs", &xs) | ra::project<1, 0>();
    std::set<ra::LineagedTuple<char, int>> expected = {
        ra::make_lineaged_tuple({{"xs", hash(t0)}}, std::make_tuple('a', 1)),
        ra::make_lineaged_tuple({{"xs", hash(t1)}}, std::make_tuple('b', 2)),
        ra::make_lineaged_tuple({{"xs", hash(t2)}}, std::make_tuple('c', 3)),
    };

    static_assert(std::is_same<decltype(project)::column_types,
                               TypeList<char, int>>::value,
                  "");
    ExpectRngsUnorderedEqual(project.ToPhysical().ToRange(), expected);
    EXPECT_EQ(project.ToDebugString(), "Project<1, 0>(xs)");
  }

  {
    auto project = ra::make_iterable("xs", &xs) | ra::project<0, 1>();
    std::set<ra::LineagedTuple<int, char>> expected = {
        ra::make_lineaged_tuple({{"xs", hash(t0)}}, t0),
        ra::make_lineaged_tuple({{"xs", hash(t1)}}, t1),
        ra::make_lineaged_tuple({{"xs", hash(t2)}}, t2),
    };

    static_assert(std::is_same<decltype(project)::column_types,
                               TypeList<int, char>>::value,
                  "");
    ExpectRngsUnorderedEqual(project.ToPhysical().ToRange(), expected);
    EXPECT_EQ(project.ToDebugString(), "Project<0, 1>(xs)");
  }
}

TEST(Project, RepeatedProject) {
  Hash<std::tuple<int, char>> hash;
  std::tuple<int, char> t0 = {1, 'a'};
  std::tuple<int, char> t1 = {2, 'b'};
  std::tuple<int, char> t2 = {3, 'c'};
  std::set<std::tuple<int, char>> xs = {t0, t1, t2};
  auto project = ra::make_iterable("xs", &xs) | ra::project<1, 0, 1>();

  std::set<ra::LineagedTuple<char, int, char>> expected = {
      ra::make_lineaged_tuple({{"xs", hash(t0)}}, std::make_tuple('a', 1, 'a')),
      ra::make_lineaged_tuple({{"xs", hash(t1)}}, std::make_tuple('b', 2, 'b')),
      ra::make_lineaged_tuple({{"xs", hash(t2)}}, std::make_tuple('c', 3, 'c')),
  };

  static_assert(std::is_same<decltype(project)::column_types,
                             TypeList<char, int, char>>::value,
                "");
  ExpectRngsUnorderedEqual(project.ToPhysical().ToRange(), expected);
  EXPECT_EQ(project.ToDebugString(), "Project<1, 0, 1>(xs)");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
