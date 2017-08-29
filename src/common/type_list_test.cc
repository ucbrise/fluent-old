#include "common/type_list.h"

#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace common {
namespace detail {

template <typename... Ts>
struct Template {};

template <typename T>
struct Default;

template <>
struct Default<int> {
  int operator()() { return 0; }
};

template <>
struct Default<char> {
  char operator()() { return 'a'; }
};

template <>
struct Default<bool> {
  char operator()() { return true; }
};

}  // namespace detail

TEST(TypeList, TypeListGet) {
  using typelist = TypeList<int, char, bool, int, float>;
  static_assert(std::is_same<int, TypeListGet<typelist, 0>::type>::value, "");
  static_assert(std::is_same<char, TypeListGet<typelist, 1>::type>::value, "");
  static_assert(std::is_same<bool, TypeListGet<typelist, 2>::type>::value, "");
  static_assert(std::is_same<int, TypeListGet<typelist, 3>::type>::value, "");
  static_assert(std::is_same<float, TypeListGet<typelist, 4>::type>::value, "");
}

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

TEST(TypeList, TypeListCons) {
  using as = TypeList<>;
  using bs = TypeListCons<int, as>::type;
  using cs = TypeListCons<bool, bs>::type;
  using ds = TypeListCons<float, cs>::type;

  static_assert(std::is_same<as, TypeList<>>::value, "");
  static_assert(std::is_same<bs, TypeList<int>>::value, "");
  static_assert(std::is_same<cs, TypeList<bool, int>>::value, "");
  static_assert(std::is_same<ds, TypeList<float, bool, int>>::value, "");
}

TEST(TypeList, TypeListAppend) {
  using as = TypeList<>;
  using bs = TypeListAppend<as, int>::type;
  using cs = TypeListAppend<bs, bool>::type;
  using ds = TypeListAppend<cs, float>::type;

  static_assert(std::is_same<as, TypeList<>>::value, "");
  static_assert(std::is_same<bs, TypeList<int>>::value, "");
  static_assert(std::is_same<cs, TypeList<int, bool>>::value, "");
  static_assert(std::is_same<ds, TypeList<int, bool, float>>::value, "");
}

TEST(TypeList, TypeListProject) {
  using xs = TypeList<int, char, float, double>;
  using ys = TypeListProject<xs, 0>::type;
  using expected = TypeList<int>;
  static_assert(std::is_same<ys, expected>::value, "");
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

TEST(TypeList, TypeListAll) {
  static_assert(TypeListAll<TypeList<>, std::is_void>::value, "");
  static_assert(TypeListAll<TypeList<void>, std::is_void>::value, "");
  static_assert(TypeListAll<TypeList<void, void>, std::is_void>::value, "");
  static_assert(!TypeListAll<TypeList<int>, std::is_void>::value, "");
  static_assert(!TypeListAll<TypeList<void, int>, std::is_void>::value, "");
  static_assert(!TypeListAll<TypeList<int, void>, std::is_void>::value, "");
}

TEST(TypeList, TypeListAllSame) {
  using a = TypeList<>;
  using b = TypeList<int>;
  using c = TypeList<int, int>;
  using d = TypeList<int, char>;
  using e = TypeList<int, int, char>;
  using f = TypeList<int, int, int>;

  static_assert(TypeListAllSame<a>::value, "");
  static_assert(TypeListAllSame<b>::value, "");
  static_assert(TypeListAllSame<c>::value, "");
  static_assert(!TypeListAllSame<d>::value, "");
  static_assert(!TypeListAllSame<e>::value, "");
  static_assert(TypeListAllSame<f>::value, "");
}

TEST(TypeList, TypeListMapToTuple) {
  using typelist = TypeList<int, char, bool>;
  EXPECT_EQ((std::tuple<int, char, bool>{0, 'a', true}),
            (TypeListMapToTuple<typelist, detail::Default>()()));
}

TEST(TypeList, TypeListTo) {
  {
    using actual = TypeListTo<detail::Template, TypeList<>>::type;
    using expected = detail::Template<>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TypeListTo<detail::Template, TypeList<int>>::type;
    using expected = detail::Template<int>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TypeListTo<detail::Template, TypeList<int, int>>::type;
    using expected = detail::Template<int, int>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TypeListTo<std::tuple, TypeList<int, int>>::type;
    using expected = std::tuple<int, int>;
    static_assert(std::is_same<actual, expected>::value, "");
  }
}

TEST(TypeList, TypeListFrom) {
  {
    using actual = TypeListFrom<detail::Template<>>::type;
    using expected = TypeList<>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TypeListFrom<detail::Template<int>>::type;
    using expected = TypeList<int>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TypeListFrom<detail::Template<int, int>>::type;
    using expected = TypeList<int, int>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TypeListFrom<std::tuple<int, int>>::type;
    using expected = TypeList<int, int>;
    static_assert(std::is_same<actual, expected>::value, "");
  }
}

TEST(TypeList, TypeListToTuple) {
  {
    using actual = TypeListToTuple<TypeList<>>::type;
    using expected = std::tuple<>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TypeListToTuple<TypeList<int>>::type;
    using expected = std::tuple<int>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TypeListToTuple<TypeList<int, int>>::type;
    using expected = std::tuple<int, int>;
    static_assert(std::is_same<actual, expected>::value, "");
  }
}

TEST(TypeList, TupleToTypleList) {
  {
    using actual = TupleToTypeList<std::tuple<>>::type;
    using expected = TypeList<>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TupleToTypeList<std::tuple<int>>::type;
    using expected = TypeList<int>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TupleToTypeList<std::tuple<int, int>>::type;
    using expected = TypeList<int, int>;
    static_assert(std::is_same<actual, expected>::value, "");
  }
}

}  // namespace common
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
