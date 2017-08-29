#include "ra/logical/filter.h"

#include <set>
#include <tuple>
#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/macros.h"
#include "ra/logical/iterable.h"

namespace lra = fluent::ra::logical;

namespace fluent {

TEST(Filter, SimpleCompileCheck) {
  std::set<std::tuple<int>> xs;
  lra::Iterable<std::set<std::tuple<int>>> i = lra::make_iterable(&xs);
  auto f = [](const std::tuple<int>&) { return true; };
  lra::Filter<decltype(i), decltype(f)> filter = i | lra::filter(f);
  UNUSED(filter);

  using actual = decltype(filter)::column_types;
  using expected = common::TypeList<int>;
  static_assert(common::StaticAssert<std::is_same<actual, expected>>::value,
                "");
}

// This code should NOT compile.
TEST(Filter, FunctionWithWrongInputArguments) {
  // std::set<std::tuple<int>> xs;
  // lra::Iterable<std::set<std::tuple<int>>> i = lra::make_iterable(&xs);
  // auto f = [](const std::tuple<std::string>&) { return std::tuple<int>(42);
  // };
  // lra::Filter<decltype(i), decltype(f)> filter = i | lra::filter(f);
}

// This code should NOT compile.
TEST(Filter, FunctionThatDoesntReturnATuple) {
  // std::set<std::tuple<int>> xs;
  // lra::Iterable<std::set<std::tuple<int>>> i = lra::make_iterable(&xs);
  // auto f = [](const std::tuple<int>&) { return 42; };
  // lra::Filter<decltype(i), decltype(f)> filter = i | lra::filter(f);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
