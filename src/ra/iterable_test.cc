#include "ra/iterable.h"

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

TEST(Relalg, EmptyIterable) {
  std::vector<std::tuple<int>> xs = {};
  auto iter = ra::make_iterable(&xs);
  static_assert(
      std::is_same<decltype(iter)::column_types, TypeList<int>>::value, "");
  ExpectRngsEqual(iter.ToPhysical().ToRange(), ranges::view::all(xs));
}

TEST(Relalg, SimpleIterable) {
  std::vector<std::tuple<int>> xs = {{1}, {2}, {3}};
  auto iter = ra::make_iterable(&xs);
  static_assert(
      std::is_same<decltype(iter)::column_types, TypeList<int>>::value, "");
  ExpectRngsEqual(iter.ToPhysical().ToRange(), ranges::view::all(xs));
}

TEST(Relalg, MapIterable) {
  std::vector<std::tuple<int, int>> xs = {{1, 1}, {1, 2}, {1, 3}, {2, 1}, {2, 2}, {2, 3}, {3, 1}, {3, 2}, {3, 3}};
  std::vector<std::tuple<int, int>> ys = {{1, 1}, {2, 2}, {3, 3}};
  std::vector<std::tuple<int, int, std::string>> zs = {{1, 1, "A"}, {1, 2, "B"}, {1, 3, "C"},
                                                       {2, 4, "D"}, {2, 5, "E"}, {2, 6, "F"},
                                                       {3, 7, "G"}, {3, 8, "H"}, {3, 9, "I"}};
  std::unordered_map<int, std::unordered_map<int, std::string>> m;
  m[1] = std::unordered_map<int, std::string>({{1, "A"}, {2, "B"}, {3, "C"}});
  m[2] = std::unordered_map<int, std::string>({{4, "D"}, {5, "E"}, {6, "F"}});
  m[3] = std::unordered_map<int, std::string>({{7, "G"}, {8, "H"}, {9, "I"}});
  // auto rng =  ranges::view::all(m) | ranges::view::for_each([m](const auto& l) {
  //               return ranges::yield_from(ranges::view::all(m) | ranges::view::filter([l](const auto& t) {
  //                                                                  return l.first == t.first;
  //                                                                })
  //                                                              | ranges::view::for_each([m, l](const auto& r) {
  //                                                                  return ranges::yield_from(ranges::view::all(m.at(r.first)) | ranges::view::filter([m, r](const auto& t) {
  //                                                                                                                              return m.at(r.first).first == t.first;
  //                                                                                                                            })
  //                                                                                                                          | ranges::view::transform([m, r](const auto& f) {
  //                                                                                                                              return std::make_tuple(r.first, f.first, f.second);
  //                                                                                                                            }));
  //                                                                }));
  //             });
  auto rng =  ranges::view::all(m) | ranges::view::for_each([](const auto& l) {
                return ranges::yield_from(ranges::view::all(l.second) | ranges::view::transform([l](const auto& r) {
                                                                          return std::make_tuple(l.first, r.first, r.second);
                                                                        }));
              });
  //(void)rng;
  // auto rng = ranges::view::all(m) | ranges::view::transform([](const auto& e) {
  //                                     return std::make_tuple(e.first, e.second);
  //                                   });
  // auto test = ranges::view::all(m) | ranges::view::keys;
  // for (auto it = ranges::begin(test); it != ranges::end(test); ++it) {
  //   std::cout << *it << "\n";
  // }
  ExpectRngsUnorderedEqual(rng, zs);
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
