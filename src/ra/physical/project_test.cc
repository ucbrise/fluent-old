#include "ra/physical/project.h"

#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/physical/iterable.h"
#include "testing/test_util.h"

namespace pra = fluent::ra::physical;

namespace fluent {

TEST(Project, EmptyProject) {
  std::set<std::tuple<int, char, bool>> xs;
  auto iterable = pra::make_iterable(&xs);
  auto project = pra::make_project<0, 1, 2>(std::move(iterable));
  std::set<std::tuple<int, char, bool>> expected;
  testing::ExpectRngsUnorderedEqual(project.ToRange(), expected);
}

TEST(Project, ZeroColumnProject) {
  std::set<std::tuple<int, char, bool>> xs = {{1, 'a', true}, {2, 'b', false}};
  auto iterable = pra::make_iterable(&xs);
  auto project = pra::make_project<>(std::move(iterable));
  std::set<std::tuple<>> expected = {{}};
  testing::ExpectRngsUnorderedEqual(project.ToRange(), expected);
}

TEST(Project, SimpleProject) {
  std::set<std::tuple<int, char, bool>> xs = {{1, 'a', true}, {2, 'b', false}};
  auto iterable = pra::make_iterable(&xs);
  auto project = pra::make_project<2, 1>(std::move(iterable));
  std::set<std::tuple<bool, char>> expected = {{true, 'a'}, {false, 'b'}};
  testing::ExpectRngsUnorderedEqual(project.ToRange(), expected);
}

TEST(Project, RepeatedColumnsProject) {
  std::set<std::tuple<int, char, bool>> xs = {{1, 'a', true}, {2, 'b', false}};
  auto iterable = pra::make_iterable(&xs);
  auto project = pra::make_project<0, 1, 2, 1, 0>(std::move(iterable));
  std::set<std::tuple<int, char, bool, char, int>> expected = {
      {1, 'a', true, 'a', 1}, {2, 'b', false, 'b', 2}};
  testing::ExpectRngsUnorderedEqual(project.ToRange(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
