#include "common/type_traits.h"

#include <memory>
#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace common {

TEST(TypeTraits, And) {
  static_assert(And<std::true_type, std::true_type>::value, "");
  static_assert(!And<std::true_type, std::false_type>::value, "");
  static_assert(!And<std::false_type, std::true_type>::value, "");
  static_assert(!And<std::false_type, std::false_type>::value, "");
}

TEST(TypeTraits, Or) {
  static_assert(Or<std::true_type, std::true_type>::value, "");
  static_assert(Or<std::true_type, std::false_type>::value, "");
  static_assert(Or<std::false_type, std::true_type>::value, "");
  static_assert(!Or<std::false_type, std::false_type>::value, "");
}

TEST(TypeTraits, Not) {
  static_assert(Not<std::false_type>::value, "");
  static_assert(!Not<std::true_type>::value, "");
}

TEST(TypeTraits, Lt) {
  using one = std::integral_constant<int, 1>;
  using two = std::integral_constant<int, 2>;
  static_assert(!Lt<one, one>::value, "");
  static_assert(Lt<one, two>::value, "");
  static_assert(!Lt<two, one>::value, "");
  static_assert(!Lt<two, two>::value, "");
}

TEST(TypeTraits, Le) {
  using one = std::integral_constant<int, 1>;
  using two = std::integral_constant<int, 2>;
  static_assert(Le<one, one>::value, "");
  static_assert(Le<one, two>::value, "");
  static_assert(!Le<two, one>::value, "");
  static_assert(Le<two, two>::value, "");
}

TEST(TypeTraits, Eq) {
  using one = std::integral_constant<int, 1>;
  using two = std::integral_constant<int, 2>;
  static_assert(Eq<one, one>::value, "");
  static_assert(!Eq<one, two>::value, "");
  static_assert(!Eq<two, one>::value, "");
  static_assert(Eq<two, two>::value, "");
}

TEST(TypeTraits, Ne) {
  using one = std::integral_constant<int, 1>;
  using two = std::integral_constant<int, 2>;
  static_assert(!Ne<one, one>::value, "");
  static_assert(Ne<one, two>::value, "");
  static_assert(Ne<two, one>::value, "");
  static_assert(!Ne<two, two>::value, "");
}

TEST(TypeTraits, Gt) {
  using one = std::integral_constant<int, 1>;
  using two = std::integral_constant<int, 2>;
  static_assert(!Gt<one, one>::value, "");
  static_assert(!Gt<one, two>::value, "");
  static_assert(Gt<two, one>::value, "");
  static_assert(!Gt<two, two>::value, "");
}

TEST(TypeTraits, Ge) {
  using one = std::integral_constant<int, 1>;
  using two = std::integral_constant<int, 2>;
  static_assert(Ge<one, one>::value, "");
  static_assert(!Ge<one, two>::value, "");
  static_assert(Ge<two, one>::value, "");
  static_assert(Ge<two, two>::value, "");
}

TEST(TypeTraits, All) {
  static_assert(All<>::value, "");
  static_assert(All<std::true_type>::value, "");
  static_assert(!All<std::false_type>::value, "");
  static_assert(All<std::true_type, std::true_type>::value, "");
  static_assert(!All<std::true_type, std::false_type>::value, "");
}

TEST(TypeTraits, Any) {
  static_assert(!Any<>::value, "");
  static_assert(Any<std::true_type>::value, "");
  static_assert(!Any<std::false_type>::value, "");
  static_assert(Any<std::true_type, std::true_type>::value, "");
  static_assert(Any<std::true_type, std::false_type>::value, "");
}

TEST(TypeTraits, InRange) {
  static_assert(InRange<0, 0, 5>::value, "");
  static_assert(InRange<1, 0, 5>::value, "");
  static_assert(InRange<2, 0, 5>::value, "");
  static_assert(InRange<3, 0, 5>::value, "");
  static_assert(InRange<4, 0, 5>::value, "");
  static_assert(!InRange<5, 0, 5>::value, "");
}

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

TEST(TypeTraits, Unwrap) {
  static_assert(std::is_same<int, Unwrap<std::unique_ptr<int>>::type>::value,
                "");
  static_assert(std::is_same<bool, Unwrap<std::unique_ptr<bool>>::type>::value,
                "");
  static_assert(std::is_same<int, Unwrap<std::tuple<int>>::type>::value, "");
  static_assert(std::is_same<bool, Unwrap<std::tuple<bool>>::type>::value, "");
  static_assert(std::is_same<int, Unwrap<std::set<int>>::type>::value, "");
  static_assert(std::is_same<bool, Unwrap<std::set<bool>>::type>::value, "");
}

TEST(TypeTraits, IsInvocable) {
  static_assert(IsInvocable<void(int), int>::value, "");
  static_assert(IsInvocable<void(const int), int>::value, "");
  static_assert(IsInvocable<void(const int&), int>::value, "");
  static_assert(IsInvocable<bool(int), int>::value, "");
  static_assert(IsInvocable<bool(const int), int>::value, "");
  static_assert(IsInvocable<bool(const int&), int>::value, "");

  static_assert(!IsInvocable<void(int, int), int>::value, "");
  static_assert(!IsInvocable<void(int&), int>::value, "");
  static_assert(!IsInvocable<void(std::string), int>::value, "");
  static_assert(!IsInvocable<void(), int>::value, "");
}

}  // namespace common
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
