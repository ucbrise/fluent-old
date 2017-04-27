#include "ra/map.h"
#include "ra/map_iterable.h"
#include "fluent/map_lattice.h"
#include "fluent/max_lattice.h"
#include "fluent/set_lattice.h"

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/type_list.h"
#include "testing/test_util.h"

namespace fluent {

TEST(MapIterable, SimpleTest) {
  std::vector<std::tuple<int, int, MaxLattice<int>>> xs = {{1, 1, 100}, {1, 2, 200}, {2, 1, 300}, {2, 2, 400}};
  std::vector<std::tuple<int, int>> ys = {{1, 1}, {1, 2}, {2, 1}, {2, 2}};
  std::vector<std::tuple<int, MaxLattice<int>>> zs = {{1, 100}, {2, 200}};

  std::unordered_map<int, MaxLattice<int>> sub1 = {{1, 100}, {2, 200}};
  std::unordered_map<int, MaxLattice<int>> sub2 = {{1, 300}, {2, 400}};
  std::unordered_map<int, MapLattice<int, MaxLattice<int>>> m = {{1, sub1}, {2, sub2}};
  MapLattice<int, MapLattice<int, MaxLattice<int>>> mapl(m);
  auto test = mapl.Iterable() | ra::map([](const auto& t) {
               return std::make_tuple(std::get<0>(t), std::get<1>(t));
             });
  ExpectRngsUnorderedEqual(mapl.Iterable().ToPhysical().ToRange(), xs);
  ExpectRngsUnorderedEqual(mapl.at(1).Iterable().ToPhysical().ToRange(), zs);
  ExpectRngsUnorderedEqual(test.ToPhysical().ToRange(), ys);
}

TEST(MapIterable, NestedSetLatticeTest) {
  std::vector<std::tuple<int, int, int>> xs = {{1, 1, 1}, {1, 2, 2}, {2, 1, 1}, {2, 2, 2}};
  std::vector<std::tuple<int, int>> ys = {{1, 1}, {1, 2}, {2, 1}, {2, 2}};
  std::vector<std::tuple<int, MaxLattice<int>>> zs = {{1, 1}, {2, 2}};

  std::set<std::tuple<int, int>> sub1 = {{1, 1}, {2, 2}};
  std::unordered_map<int, SetLattice<int, int>> m = {{1, sub1}, {2, sub1}};
  MapLattice<int, SetLattice<int, int>> mapl(m);
  auto test = mapl.Iterable() | ra::map([](const auto& t) {
               return std::make_tuple(std::get<0>(t), std::get<1>(t));
             });
  ExpectRngsUnorderedEqual(mapl.Iterable().ToPhysical().ToRange(), xs);
  ExpectRngsUnorderedEqual(mapl.at(1).Iterable().ToPhysical().ToRange(), zs);
  ExpectRngsUnorderedEqual(test.ToPhysical().ToRange(), ys);
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
