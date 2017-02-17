#include "common/type_list.h"

#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(TypeList, TypeListMap) {
  using xs = TypeList<int, char, float>;
  using ys = TypeListMap<xs, std::add_pointer>::type;
  using zs = TypeListMap<ys, std::remove_pointer>::type;

  static_assert(std::is_same<ys, TypeList<int*, char*, float*>>::value, "");
  static_assert(std::is_same<zs, xs>::value, "");
}

TEST(TypeList, TypeListConcat) {
  using xs = TypeList<int, char>;
  using ys = TypeList<float, double>;
  using zs = TypeListConcat<xs, ys>::type;
  static_assert(std::is_same<zs, TypeList<int, char, float, double>>::value,
                "");
}

TEST(TypeList, TypeListProject) {
  using xs = TypeList<int, char, float, double>;

  {
    using ys = TypeListProject<xs, 0>::type;
    using expected = TypeList<int>;
    static_assert(std::is_same<ys, expected>::value, "");
  }
}

TEST(TypeList, TypeListTake) {
  using tl = TypeList<int, char, float, double>;
  using zero = TypeList<>;
  using one = TypeList<int>;
  using two = TypeList<int, char>;
  using three = TypeList<int, char, float>;
  using four = TypeList<int, char, float, double>;

  static_assert(std::is_same<TypeListTake<tl, 0>::type, zero>::value, "");
  static_assert(std::is_same<TypeListTake<tl, 1>::type, one>::value, "");
  static_assert(std::is_same<TypeListTake<tl, 2>::type, two>::value, "");
  static_assert(std::is_same<TypeListTake<tl, 3>::type, three>::value, "");
  static_assert(std::is_same<TypeListTake<tl, 4>::type, four>::value, "");
  static_assert(std::is_same<TypeListTake<tl, 5>::type, four>::value, "");
}

TEST(TypeList, TypeListDrop) {
  using tl = TypeList<int, char, float, double>;
  using zero = TypeList<int, char, float, double>;
  using one = TypeList<char, float, double>;
  using two = TypeList<float, double>;
  using three = TypeList<double>;
  using four = TypeList<>;

  static_assert(std::is_same<TypeListDrop<tl, 0>::type, zero>::value, "");
  static_assert(std::is_same<TypeListDrop<tl, 1>::type, one>::value, "");
  static_assert(std::is_same<TypeListDrop<tl, 2>::type, two>::value, "");
  static_assert(std::is_same<TypeListDrop<tl, 3>::type, three>::value, "");
  static_assert(std::is_same<TypeListDrop<tl, 4>::type, four>::value, "");
  static_assert(std::is_same<TypeListDrop<tl, 5>::type, four>::value, "");
}

TEST(TypeList, TypeListLen) {
  using zero = TypeList<>;
  using one = TypeList<int>;
  using two = TypeList<int, char>;
  using three = TypeList<int, char, float>;
  using four = TypeList<int, char, float, double>;

  static_assert(TypeListLen<zero>::value == 0, "");
  static_assert(TypeListLen<one>::value == 1, "");
  static_assert(TypeListLen<two>::value == 2, "");
  static_assert(TypeListLen<three>::value == 3, "");
  static_assert(TypeListLen<four>::value == 4, "");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
