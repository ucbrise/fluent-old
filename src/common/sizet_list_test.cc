#include "common/sizet_list.h"

#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace detail {

template <std::size_t... Is>
struct Template {};

}  // namespace detail

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

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
