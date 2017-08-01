#include "ra/physical/iterable.h"

#include <set>
#include <tuple>
#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "testing/test_util.h"

namespace pra = fluent::ra::physical;

namespace fluent {

TEST(Relalg, EmptyIterable) {
  std::set<std::tuple<int>> xs = {};
  pra::Iterable<std::set<std::tuple<int>>> iter = pra::make_iterable(&xs);
  std::set<std::tuple<int>> expected = {};
  testing::ExpectRngsUnorderedEqual(iter.ToRange(), expected);
}

TEST(Relalg, NonEmptyIterable) {
  std::set<std::tuple<int>> xs = {{1}, {2}, {3}};
  pra::Iterable<std::set<std::tuple<int>>> iter = pra::make_iterable(&xs);
  std::set<std::tuple<int>> expected = {{1}, {2}, {3}};
  testing::ExpectRngsUnorderedEqual(iter.ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
