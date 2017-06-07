#include "ra/physical/cross.h"

#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/physical/iterable.h"
#include "testing/test_util.h"

namespace pra = fluent::ra::physical;

namespace fluent {

TEST(Cross, EmptyCross) {
  std::set<std::tuple<int>> xs;
  std::set<std::tuple<char>> ys;
  auto iterable_xs = pra::make_iterable(&xs);
  auto iterable_ys = pra::make_iterable(&ys);
  auto cross = pra::make_cross(std::move(iterable_xs), std::move(iterable_ys));
  std::set<std::tuple<int, char>> expected;
  ExpectRngsUnorderedEqual(cross.ToRange(), expected);
}

TEST(Cross, NonEmptyCross) {
  std::set<std::tuple<int>> xs = {1, 2, 3};
  std::set<std::tuple<char>> ys = {'a', 'b', 'c'};
  auto iterable_xs = pra::make_iterable(&xs);
  auto iterable_ys = pra::make_iterable(&ys);
  auto cross = pra::make_cross(std::move(iterable_xs), std::move(iterable_ys));
  std::set<std::tuple<int, char>> expected = {{1, 'a'}, {1, 'b'}, {1, 'c'},
                                              {2, 'a'}, {2, 'b'}, {2, 'c'},
                                              {3, 'a'}, {3, 'b'}, {3, 'c'}};
  ExpectRngsUnorderedEqual(cross.ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
