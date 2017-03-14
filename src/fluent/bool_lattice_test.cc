#include "fluent/bool_lattice.h"

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

TEST(BoolLattice, SimpleMerge) {
  //TODO(cgwu) check back no this
  BoolLattice l("l", false);
  EXPECT_THAT(l.Reveal(), false);
  
  std::set<std::tuple<bool>> s1 = {false, true, false};

  l.Merge(ra::make_iterable(&s1));
  EXPECT_THAT(l.Reveal(), true);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
