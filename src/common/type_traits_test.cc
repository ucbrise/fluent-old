#include "common/type_traits.h"

#include <set>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(TypeTraits, IsSet) {
  static_assert(IsSet<std::set<int>>::value, "");
  static_assert(IsSet<std::set<bool>>::value, "");
  static_assert(IsSet<std::set<std::set<int>>>::value, "");
  static_assert(IsSet<std::set<std::set<int>>>::value, "");

  static_assert(!IsSet<int>::value, "");
  static_assert(!IsSet<bool>::value, "");
  static_assert(!IsSet<std::set<int>&>::value, "");
  static_assert(!IsSet<const std::set<int>>::value, "");
  static_assert(!IsSet<const std::set<int>&>::value, "");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
