#include "common/type_traits.h"

#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(TypeTraits, IsTemplate) {
  static_assert(IsTemplate<std::set, std::set<int>>::value, "");
  static_assert(IsTemplate<std::vector, std::vector<int>>::value, "");

  static_assert(!IsTemplate<std::vector, int>::value, "");
  static_assert(!IsTemplate<std::vector, std::set<int>>::value, "");
  static_assert(!IsTemplate<std::vector, std::vector<int>&>::value, "");
  static_assert(!IsTemplate<std::vector, const std::vector<int>&>::value, "");
}

TEST(TypeTraits, IsSet) {
  static_assert(IsSet<std::set<int>>::value, "");
  static_assert(IsSet<std::set<bool>>::value, "");
  static_assert(IsSet<std::set<std::set<int>>>::value, "");

  static_assert(!IsSet<int>::value, "");
  static_assert(!IsSet<bool>::value, "");
  static_assert(!IsSet<std::set<int>&>::value, "");
  static_assert(!IsSet<const std::set<int>>::value, "");
  static_assert(!IsSet<const std::set<int>&>::value, "");
}

TEST(TypeTraits, IsVector) {
  static_assert(IsVector<std::vector<int>>::value, "");
  static_assert(IsVector<std::vector<bool>>::value, "");
  static_assert(IsVector<std::vector<std::vector<int>>>::value, "");

  static_assert(!IsVector<int>::value, "");
  static_assert(!IsVector<bool>::value, "");
  static_assert(!IsVector<std::vector<int>&>::value, "");
  static_assert(!IsVector<const std::vector<int>>::value, "");
  static_assert(!IsVector<const std::vector<int>&>::value, "");
}

TEST(TypeTraits, IsTuple) {
  static_assert(IsTuple<std::tuple<>>::value, "");
  static_assert(IsTuple<std::tuple<int>>::value, "");
  static_assert(IsTuple<std::tuple<int, bool>>::value, "");
  static_assert(IsTuple<std::tuple<int, bool, char>>::value, "");
  static_assert(IsTuple<std::tuple<std::tuple<int>>>::value, "");
  static_assert(IsTuple<std::tuple<int, std::tuple<int>>>::value, "");

  static_assert(!IsTuple<int>::value, "");
  static_assert(!IsTuple<bool>::value, "");
  static_assert(!IsTuple<std::tuple<int>&>::value, "");
  static_assert(!IsTuple<const std::tuple<int>>::value, "");
  static_assert(!IsTuple<const std::tuple<int>&>::value, "");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
