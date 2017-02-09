#include "fluent/table.h"

#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ra/all.h"

using ::testing::UnorderedElementsAreArray;

namespace fluent {

TEST(Table, SimpleTest) {
  Table<int, int, int> t("t");

  using Tuple = std::tuple<int, int, int>;
  using TupleSet = std::set<Tuple>;
  Tuple x1{1, 1, 1};
  Tuple x2{2, 2, 2};
  Tuple x3{3, 3, 3};

  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(TupleSet{}));
  t.Add(x1);
  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(TupleSet{x1}));
  t.Add(x2);
  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(TupleSet{x1, x2}));
  t.Add(x3);
  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(TupleSet{x1, x2, x3}));
  t.Tick();
  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(TupleSet{x1, x2, x3}));
}

TEST(Table, SimpleQuery) {
  using Tuple = std::tuple<int, int, int>;
  using TupleSet = std::set<Tuple>;

  Table<int, int, int> t("t");
  for (int i = 0; i < 10; ++i) {
    t.Add({i, i, i});
  }

  auto all_eq = [](const Tuple& t) {
    return std::get<0>(t) == std::get<1>(t) && std::get<1>(t) == std::get<2>(t);
  };
  auto times_two = [](const Tuple& t) {
    int doubled = std::get<0>(t) * 2;
    return Tuple{doubled, doubled, doubled};
  };
  auto not_too_big = [](const Tuple& t) {
    return std::get<0>(t) + std::get<1>(t) + std::get<2>(t) < 50;
  };
  // clang-format off
  t.AddRelalg(t.Iterable()
    | ra::filter(std::move(all_eq))
    | ra::map(std::move(times_two))
    | ra::filter(std::move(not_too_big)));
  // clang-format on

  TupleSet expected;
  for (int i = 0; i < 10; ++i) {
    expected.insert({i, i, i});
    const int doubled = i * 2;
    if (doubled * 3 < 50) {
      expected.insert({doubled, doubled, doubled});
    }
  }
  EXPECT_THAT(t.Get(), UnorderedElementsAreArray(expected));
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
