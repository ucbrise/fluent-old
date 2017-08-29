#include "ra/logical/iterable.h"

#include <set>
#include <tuple>
#include <type_traits>
#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/macros.h"
#include "common/static_assert.h"

namespace lra = fluent::ra::logical;

namespace fluent {

TEST(Iterable, SimpleCompileCheckSet) {
  std::set<std::tuple<int>> xs;
  lra::Iterable<std::set<std::tuple<int>>> iterable = lra::make_iterable(&xs);
  UNUSED(iterable);

  using actual = decltype(iterable)::column_types;
  using expected = common::TypeList<int>;
  static_assert(common::StaticAssert<std::is_same<actual, expected>>::value,
                "");
}

TEST(Iterable, SimpleCompileCheckVector) {
  std::vector<std::tuple<int>> xs;
  lra::Iterable<std::vector<std::tuple<int>>> iterable =
      lra::make_iterable(&xs);
  UNUSED(iterable);

  using column_types = decltype(iterable)::column_types;
  using is_same = std::is_same<column_types, common::TypeList<int>>;
  static_assert(common::StaticAssert<is_same>::value, "");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
