#include "ra/physical/filter.h"

#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/physical/iterable.h"
#include "testing/test_util.h"

namespace pra = fluent::ra::physical;

namespace fluent {

TEST(Filter, EmptyFilter) {
  std::set<std::tuple<int>> xs;
  auto iterable = pra::make_iterable(&xs);
  auto f = [](const std::tuple<int>&) { return true; };
  auto filter = pra::make_filter(std::move(iterable), f);
  std::set<std::tuple<int>> expected;
  ExpectRngsUnorderedEqual(filter.ToRange(), expected);
}

TEST(Filter, NonEmptyFilter) {
  std::set<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto iterable = pra::make_iterable(&xs);
  auto f = [](const std::tuple<int>& t) { return std::get<0>(t) % 2 == 1; };
  auto filter = pra::make_filter(std::move(iterable), f);
  std::set<std::tuple<int>> expected = {{1}, {3}};
  ExpectRngsUnorderedEqual(filter.ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
