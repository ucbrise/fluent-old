#include "ra/batch.h"
#include "ra/iterable.h"
#include "ra/unbatch.h"

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include <unordered_map>

#include "common/type_list.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Relalg, SimpleUnbatchTest) {
  std::set<std::tuple<std::string, int>> xs = {{"A", 1}, {"B", 2}};
  std::set<std::tuple<std::set<std::tuple<std::string, int>>>> ys;
  ys.insert(std::make_tuple(xs));
  auto relalg = ra::make_iterable(&ys) | ra::unbatch();
  ExpectRngsUnorderedEqual(relalg.ToPhysical().ToRange(), xs);
}

TEST(Relalg, BatchUnbatchTest) {
  std::set<std::tuple<std::string, int>> xs = {{"A", 1}, {"B", 2}};
  auto relalg = ra::make_iterable(&xs) | ra::batch() | ra::unbatch();
  ExpectRngsUnorderedEqual(relalg.ToPhysical().ToRange(), xs);
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
