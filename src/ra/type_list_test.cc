#include "ra/type_list.h"

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

TEST(TypeList, TypeListMap) {
  using xs = ra::TypeList<int, char, float>;
  using ys = ra::TypeListMap<xs, std::add_pointer>::type;
  using zs = ra::TypeListMap<ys, std::remove_pointer>::type;

  static_assert(std::is_same<ys, ra::TypeList<int*, char*, float*>>::value, "");
  static_assert(std::is_same<zs, xs>::value, "");
}

TEST(TypeList, TypeListConcat) {
  using xs = ra::TypeList<int, char>;
  using ys = ra::TypeList<float, double>;
  using zs = ra::TypeListConcat<xs, ys>::type;
  static_assert(std::is_same<zs, ra::TypeList<int, char, float, double>>::value,
                "");
}

TEST(TypeList, TypeListProject) {
  using xs = ra::TypeList<int, char, float, double>;

  {
    using ys = ra::TypeListProject<xs, 0>::type;
    using expected = ra::TypeList<int>;
    static_assert(std::is_same<ys, expected>::value, "");
  }
}

TEST(TypeList, TypeListTake) {
  using tl = ra::TypeList<int, char, float, double>;
  using zero = ra::TypeList<>;
  using one = ra::TypeList<int>;
  using two = ra::TypeList<int, char>;
  using three = ra::TypeList<int, char, float>;
  using four = ra::TypeList<int, char, float, double>;

  static_assert(std::is_same<ra::TypeListTake<tl, 0>::type, zero>::value, "");
  static_assert(std::is_same<ra::TypeListTake<tl, 1>::type, one>::value, "");
  static_assert(std::is_same<ra::TypeListTake<tl, 2>::type, two>::value, "");
  static_assert(std::is_same<ra::TypeListTake<tl, 3>::type, three>::value, "");
  static_assert(std::is_same<ra::TypeListTake<tl, 4>::type, four>::value, "");
  static_assert(std::is_same<ra::TypeListTake<tl, 5>::type, four>::value, "");
}

TEST(TypeList, TypeListDrop) {
  using tl = ra::TypeList<int, char, float, double>;
  using zero = ra::TypeList<int, char, float, double>;
  using one = ra::TypeList<char, float, double>;
  using two = ra::TypeList<float, double>;
  using three = ra::TypeList<double>;
  using four = ra::TypeList<>;

  static_assert(std::is_same<ra::TypeListDrop<tl, 0>::type, zero>::value, "");
  static_assert(std::is_same<ra::TypeListDrop<tl, 1>::type, one>::value, "");
  static_assert(std::is_same<ra::TypeListDrop<tl, 2>::type, two>::value, "");
  static_assert(std::is_same<ra::TypeListDrop<tl, 3>::type, three>::value, "");
  static_assert(std::is_same<ra::TypeListDrop<tl, 4>::type, four>::value, "");
  static_assert(std::is_same<ra::TypeListDrop<tl, 5>::type, four>::value, "");
}

TEST(TypeList, TypeListLen) {
  using zero = ra::TypeList<>;
  using one = ra::TypeList<int>;
  using two = ra::TypeList<int, char>;
  using three = ra::TypeList<int, char, float>;
  using four = ra::TypeList<int, char, float, double>;

  static_assert(ra::TypeListLen<zero>::value == 0, "");
  static_assert(ra::TypeListLen<one>::value == 1, "");
  static_assert(ra::TypeListLen<two>::value == 2, "");
  static_assert(ra::TypeListLen<three>::value == 3, "");
  static_assert(ra::TypeListLen<four>::value == 4, "");
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
