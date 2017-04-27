#include "fluent/set_lattice.h"

#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include "common/function_traits.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ra/all.h"

#include "testing/test_util.h"

using ::testing::UnorderedElementsAreArray;

namespace fluent {

TEST(SetLattice, SimpleMerge) {
  std::set<std::tuple<int, int>> s1 = {{1, 5}, {1, 4}};
  SetLattice<int, int> setl("setl");

  setl.Merge(ra::make_iterable(&s1) | ra::map([](const auto& t) {
                                        SetLattice<int, int> l;
                                        l.merge(t);
                                        return std::make_tuple(l);
                                      }));
  std::set<std::tuple<int, int>> res = setl.Reveal();
  ExpectRngsUnorderedEqual(res, s1);
}

TEST(SetLattice, HasIterableCheck) {
  bool res = fluent::has_Iterable<SetLattice<int, int>>::value;
  EXPECT_THAT(res, true);
  res = fluent::has_Iterable<std::set<std::tuple<int, int>>>::value;
  EXPECT_THAT(res, false);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
