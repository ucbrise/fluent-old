#include "ra/project.h"

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/iterable.h"
#include "ra/type_list.h"
#include "testing/test_util.h"

namespace fluent {

template <typename T>
void type() {
  std::cout << __PRETTY_FUNCTION__ << std::endl;
}

TEST(Project, SimpleProject) {
  std::vector<std::tuple<int, char>> xs = {{1, 'a'}, {2, 'b'}, {3, 'c'}};

  {
    auto project = ra::make_iterable(&xs) | ra::project<0>();
    static_assert(
        std::is_same<decltype(project)::column_types, ra::TypeList<int>>::value,
        "");
    std::vector<std::tuple<int>> expected = {{1}, {2}, {3}};
    ExpectRngsEqual(project.ToPhysical().ToRange(),
                    ranges::view::all(expected));
  }

  {
    auto project = ra::make_iterable(&xs) | ra::project<1>();
    std::vector<std::tuple<int>> expected = {{'a'}, {'b'}, {'c'}};
    static_assert(std::is_same<decltype(project)::column_types,
                               ra::TypeList<char>>::value,
                  "");
    ExpectRngsEqual(project.ToPhysical().ToRange(),
                    ranges::view::all(expected));
  }

  {
    auto project = ra::make_iterable(&xs) | ra::project<1, 0>();
    static_assert(std::is_same<decltype(project)::column_types,
                               ra::TypeList<char, int>>::value,
                  "");
    std::vector<std::tuple<char, int>> expected = {
        {'a', 1}, {'b', 2}, {'c', 3}};
    ExpectRngsEqual(project.ToPhysical().ToRange(),
                    ranges::view::all(expected));
  }

  {
    auto project = ra::make_iterable(&xs) | ra::project<0, 1>();
    static_assert(std::is_same<decltype(project)::column_types,
                               ra::TypeList<int, char>>::value,
                  "");
    ExpectRngsEqual(project.ToPhysical().ToRange(), ranges::view::all(xs));
  }
}

TEST(Project, RepeatedProject) {
  std::vector<std::tuple<int, char>> xs = {{1, 'a'}, {2, 'b'}, {3, 'c'}};
  auto project = ra::make_iterable(&xs) | ra::project<1, 0, 1>();
  static_assert(std::is_same<decltype(project)::column_types,
                             ra::TypeList<char, int, char>>::value,
                "");
  std::vector<std::tuple<char, int, char>> expected = {
      {'a', 1, 'a'}, {'b', 2, 'b'}, {'c', 3, 'c'}};
  ExpectRngsEqual(project.ToPhysical().ToRange(), ranges::view::all(expected));
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
