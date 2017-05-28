#include "ra/logical/map.h"

#include <set>
#include <tuple>
#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/macros.h"
#include "ra/logical/iterable.h"

namespace lra = fluent::ra::logical;

namespace fluent {

TEST(Map, SimpleCompileCheck) {
  std::set<std::tuple<int>> xs;
  lra::Iterable<std::set<std::tuple<int>>> i = lra::make_iterable(&xs);
  auto f = [](const std::tuple<int>& x) { return x; };
  lra::Map<decltype(i), decltype(f)> map = i | lra::map(f);
  UNUSED(map);

  using actual = decltype(map)::column_types;
  using expected = TypeList<int>;
  static_assert(StaticAssert<std::is_same<actual, expected>>::value, "");
}

TEST(Map, FunctionsReturningTuplesWithModifiers) {
  std::set<std::tuple<int>> xs;
  lra::Iterable<std::set<std::tuple<int>>> i = lra::make_iterable(&xs);

  {
    auto f = [](const std::tuple<int>&) -> const std::tuple<int, bool> {
      return {1, true};
    };
    lra::Map<decltype(i), decltype(f)> map = i | lra::map(f);
    UNUSED(map);

    using actual = decltype(map)::column_types;
    using expected = TypeList<int, bool>;
    static_assert(StaticAssert<std::is_same<actual, expected>>::value, "");
  }

  {
    auto f = [](const std::tuple<int>& t) -> const std::tuple<int>& {
      return t;
    };
    lra::Map<decltype(i), decltype(f)> map = i | lra::map(f);
    UNUSED(map);

    using actual = decltype(map)::column_types;
    using expected = TypeList<int>;
    static_assert(StaticAssert<std::is_same<actual, expected>>::value, "");
  }
}

// This code should NOT compile.
TEST(Map, FunctionWithWrongInputArguments) {
  // std::set<std::tuple<int>> xs;
  // lra::Iterable<std::set<std::tuple<int>>> i = lra::make_iterable(&xs);
  // auto f = [](const std::tuple<std::string>&) {return std::tuple<int>(42);};
  // lra::Map<decltype(i), decltype(f)> map = i | lra::map(f);
}

// This code should NOT compile.
TEST(Map, FunctionThatDoesntReturnATuple) {
  // std::set<std::tuple<int>> xs;
  // lra::Iterable<std::set<std::tuple<int>>> i = lra::make_iterable(&xs);
  // auto f = [](const std::tuple<int>&) { return 42; };
  // lra::Map<decltype(i), decltype(f)> map = i | lra::map(f);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
