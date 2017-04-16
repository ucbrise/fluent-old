#include "fluent/lww_lattice.h"

#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ra/all.h"

using ::testing::UnorderedElementsAreArray;

namespace fluent {

TEST(LwwLattice, SimpleMerge) {
  LwwLattice<int> l("l");
  EXPECT_THAT(std::get<0>(l.Reveal()), 0);
  EXPECT_THAT(std::get<1>(l.Reveal()), 0);

  LwwLattice<int> o("o", std::pair<int, int>(10, 10));

  std::pair<int, int> p1(8, 5);
  std::pair<int, int> p2(4, 3);
  std::set<std::tuple<std::pair<int, int>>> s = {};
  s.insert(std::make_tuple(p1));
  s.insert(std::make_tuple(p2));

  l.Merge(ra::make_iterable(&s));
  EXPECT_THAT(std::get<0>(l.Reveal()), 8);
  EXPECT_THAT(std::get<1>(l.Reveal()), 5);

  l.Merge(o);
  EXPECT_THAT(std::get<0>(l.Reveal()), 10);
  EXPECT_THAT(std::get<1>(l.Reveal()), 10);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
