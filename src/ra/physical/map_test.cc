#include "ra/physical/map.h"

#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/physical/iterable.h"
#include "testing/test_util.h"

namespace pra = fluent::ra::physical;

namespace fluent {

TEST(Map, EmptyMap) {
  std::set<std::tuple<int>> xs;
  auto iterable = pra::make_iterable(&xs);
  auto f = [](const std::tuple<int>& t) { return t; };
  auto map = pra::make_map(std::move(iterable), f);
  std::set<std::tuple<int>> expected;
  testing::ExpectRngsUnorderedEqual(map.ToRange(), expected);
}

TEST(Map, NonEmptyMap) {
  std::set<std::tuple<int>> xs = {{0}, {1}, {2}};
  auto iterable = pra::make_iterable(&xs);
  auto f = [](const std::tuple<int>& t) { return std::tuple_cat(t, t); };
  auto map = pra::make_map(std::move(iterable), f);
  std::set<std::tuple<int, int>> expected = {{0, 0}, {1, 1}, {2, 2}};
  testing::ExpectRngsUnorderedEqual(map.ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
