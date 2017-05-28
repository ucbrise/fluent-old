#include "common/sizet_list.h"

#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/type_list.h"
#include "common/type_traits.h"

namespace fluent {
namespace detail {

template <std::size_t... Is>
struct Template {};

}  // namespace detail

TEST(SizetList, SizetListToTypeList) {
  {
    using actual = SizetListToTypeList<SizetList<>>::type;
    using expected = TypeList<>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = SizetListToTypeList<SizetList<1>>::type;
    using expected = TypeList<std::integral_constant<std::size_t, 1>>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = SizetListToTypeList<SizetList<1, 2>>::type;
    using expected = TypeList<std::integral_constant<std::size_t, 1>,
                              std::integral_constant<std::size_t, 2>>;
    static_assert(std::is_same<actual, expected>::value, "");
  }
}

TEST(SizetList, TypeListToSizetList) {
  {
    using actual = TypeListToSizetList<TypeList<>>::type;
    using expected = SizetList<>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TypeListToSizetList<TypeList<sizet_constant<1>>>::type;
    using expected = SizetList<1>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = TypeListToSizetList<
        TypeList<sizet_constant<1>, sizet_constant<2>>>::type;
    using expected = SizetList<1, 2>;
    static_assert(std::is_same<actual, expected>::value, "");
  }
}

TEST(SizetList, SizetListTo) {
  {
    using actual = SizetListTo<detail::Template, SizetList<>>::type;
    using expected = detail::Template<>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = SizetListTo<detail::Template, SizetList<0>>::type;
    using expected = detail::Template<0>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = SizetListTo<detail::Template, SizetList<0, 1>>::type;
    using expected = detail::Template<0, 1>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = SizetListTo<detail::Template, SizetList<0, 1, 2>>::type;
    using expected = detail::Template<0, 1, 2>;
    static_assert(std::is_same<actual, expected>::value, "");
  }
}

TEST(SizetList, SizetListFrom) {
  {
    using actual = SizetListFrom<detail::Template<>>::type;
    using expected = SizetList<>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = SizetListFrom<detail::Template<0>>::type;
    using expected = SizetList<0>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = SizetListFrom<detail::Template<0, 1>>::type;
    using expected = SizetList<0, 1>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = SizetListFrom<detail::Template<0, 1, 2>>::type;
    using expected = SizetList<0, 1, 2>;
    static_assert(std::is_same<actual, expected>::value, "");
  }
}

TEST(SizetList, SizetListFromIndexSequence) {
  {
    using actual = SizetListFromIndexSequence<std::index_sequence<>>::type;
    using expected = SizetList<>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = SizetListFromIndexSequence<std::index_sequence<0>>::type;
    using expected = SizetList<0>;
    static_assert(std::is_same<actual, expected>::value, "");
  }

  {
    using actual = SizetListFromIndexSequence<std::index_sequence<0, 1>>::type;
    using expected = SizetList<0, 1>;
    static_assert(std::is_same<actual, expected>::value, "");
  }
}

TEST(SizetList, SizetListTake) {
  using xs = SizetList<0, 1, 2, 3>;
  using zero = SizetList<>;
  using one = SizetList<0>;
  using two = SizetList<0, 1>;
  using three = SizetList<0, 1, 2>;
  using four = SizetList<0, 1, 2, 3>;

  static_assert(std::is_same<SizetListTake<xs, 0>::type, zero>::value, "");
  static_assert(std::is_same<SizetListTake<xs, 1>::type, one>::value, "");
  static_assert(std::is_same<SizetListTake<xs, 2>::type, two>::value, "");
  static_assert(std::is_same<SizetListTake<xs, 3>::type, three>::value, "");
  static_assert(std::is_same<SizetListTake<xs, 4>::type, four>::value, "");
  static_assert(std::is_same<SizetListTake<xs, 5>::type, four>::value, "");
}

TEST(SizetList, SizetListDrop) {
  using xs = SizetList<0, 1, 2, 3>;
  using zero = SizetList<0, 1, 2, 3>;
  using one = SizetList<1, 2, 3>;
  using two = SizetList<2, 3>;
  using three = SizetList<3>;
  using four = SizetList<>;

  static_assert(std::is_same<SizetListDrop<xs, 0>::type, zero>::value, "");
  static_assert(std::is_same<SizetListDrop<xs, 1>::type, one>::value, "");
  static_assert(std::is_same<SizetListDrop<xs, 2>::type, two>::value, "");
  static_assert(std::is_same<SizetListDrop<xs, 3>::type, three>::value, "");
  static_assert(std::is_same<SizetListDrop<xs, 4>::type, four>::value, "");
  static_assert(std::is_same<SizetListDrop<xs, 5>::type, four>::value, "");
}

TEST(SizetList, SizetListRange) {
  using sl00 = SizetList<>;
  using sl01 = SizetList<0>;
  using sl02 = SizetList<0, 1>;
  using sl03 = SizetList<0, 1, 2>;
  using sl13 = SizetList<1, 2>;
  using sl23 = SizetList<2>;
  using sl33 = SizetList<>;
  using sl43 = SizetList<>;

  static_assert(std::is_same<SizetListRange<0, 0>::type, sl00>::value, "");
  static_assert(std::is_same<SizetListRange<0, 1>::type, sl01>::value, "");
  static_assert(std::is_same<SizetListRange<0, 2>::type, sl02>::value, "");
  static_assert(std::is_same<SizetListRange<0, 3>::type, sl03>::value, "");
  static_assert(std::is_same<SizetListRange<1, 3>::type, sl13>::value, "");
  static_assert(std::is_same<SizetListRange<2, 3>::type, sl23>::value, "");
  static_assert(std::is_same<SizetListRange<3, 3>::type, sl33>::value, "");
  static_assert(std::is_same<SizetListRange<4, 3>::type, sl43>::value, "");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
