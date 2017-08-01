
#include "ra/physical/flat_map.h"

#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/physical/iterable.h"
#include "testing/test_util.h"

namespace pra = fluent::ra::physical;

namespace fluent {

TEST(FlatMap, EmptyFlatMapSource) {
  std::set<std::tuple<int>> xs;
  auto iterable = pra::make_iterable(&xs);
  using ret = std::vector<std::tuple<int>>;
  auto f = [](const std::tuple<int>& t) -> ret { return {t, t, t}; };
  auto flat_map = pra::make_flat_map<ret>(std::move(iterable), f);
  std::set<std::tuple<int>> expected;
  testing::ExpectRngsUnorderedEqual(flat_map.ToRange(), expected);
}

TEST(FlatMap, EmptyFlatMapReturn) {
  std::set<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto iterable = pra::make_iterable(&xs);
  using ret = std::vector<std::tuple<int>>;
  auto f = [](const std::tuple<int>&) -> ret { return {}; };
  auto flat_map = pra::make_flat_map<ret>(std::move(iterable), f);
  std::set<std::tuple<int>> expected;
  testing::ExpectRngsUnorderedEqual(flat_map.ToRange(), expected);
}

TEST(FlatMap, NonEmptyFlatMap) {
  std::set<std::tuple<int>> xs = {{0}, {1}, {2}};
  auto iterable = pra::make_iterable(&xs);
  using ret = std::vector<std::tuple<int>>;
  auto f = [](const std::tuple<int>& t) -> ret { return {t, t, t}; };
  auto flat_map = pra::make_flat_map<ret>(std::move(iterable), f);
  std::set<std::tuple<int>> expected = {{0}, {0}, {0}, {1}, {1},
                                        {1}, {2}, {2}, {2}};
  testing::ExpectRngsUnorderedEqual(flat_map.ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
