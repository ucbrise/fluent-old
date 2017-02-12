#include "relalg/project.h"

#include <tuple>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "testing/test_util.h"

namespace fluent {

TEST(GroupBy, Id) {
  //std::vector<int> xs = {0, 0, 1, 2, 2, 2, 3, 4, 5, 5};
  std::vector<int> xs = {0, 1, 2, 2, 2, 4, 5, 5, 0, 3};
  auto grouped1 = ranges::view::all(xs) | ranges::action::sort([](int x, int y) {return x < y;}) | ranges::view::group_by([](int x, int y) { return x == y; })
                    | ranges::view::transform([](auto x) {
                        std::vector<int> v = x | ranges::to_<std::vector<int>>();
                        int sum = 0;
                        for (auto& n: v)
                          sum += n;
                        return sum; 
                      });
  // ExpectRngsEqual(ranges::view::all(xs0), ranges::view::all(xs));
  // auto grouped2 = ranges::view::all(xs) | ranges::copy | ranges::action::sort(std::greater<int>()) | ranges::view::group_by([](int x, int y) { return x == y; })
  //                   | ranges::view::transform([](auto x) {
  //                       std::vector<int> v = x | ranges::to_<std::vector<int>>();
  //                       int sum = 0;
  //                       for (auto& n: v)
  //                         sum += n;
  //                       return sum; 
  //                     });
  std::vector<int> ys = {0, 1, 6, 3, 4, 10};
  // std::vector<int> zs = {10, 4, 3, 6, 1, 0};
  ExpectRngsEqual(grouped1, ranges::view::all(ys));
  //ExpectRngsEqual(grouped2, ranges::view::all(zs));
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
