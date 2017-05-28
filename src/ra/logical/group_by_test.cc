#include "ra/logical/group_by.h"

#include <set>
#include <tuple>
#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/macros.h"
#include "ra/aggregates.h"
#include "ra/keys.h"
#include "ra/logical/iterable.h"

namespace ra = fluent::ra;
namespace lra = fluent::ra::logical;

namespace fluent {

TEST(Map, SimpleCompileCheck) {
  std::set<std::tuple<int, float, float>> xs;
  lra::Iterable<std::set<std::tuple<int, float, float>>> i =
      lra::make_iterable(&xs);
  lra::GroupBy<decltype(i), ra::Keys<0>, ra::agg::Sum<1, 2>> group_by =
      i | lra::group_by<ra::Keys<0>, ra::agg::Sum<1, 2>>();
  UNUSED(group_by);

  using actual = decltype(group_by)::column_types;
  using expected = TypeList<int, float>;
  static_assert(StaticAssert<std::is_same<actual, expected>>::value, "");
}

// This code should NOT compile.
TEST(Map, KeyOutOfRange) {
  // std::set<std::tuple<int, float, float>> xs;
  // lra::Iterable<std::set<std::tuple<int, float, float>>> i =
  //     lra::make_iterable(&xs);
  // lra::GroupBy<decltype(i), ra::Keys<4>, ra::agg::Sum<1, 2>> group_by =
  //     i | lra::group_by<ra::Keys<4>, ra::agg::Sum<1, 2>>();
}

// This code should NOT compile.
TEST(Map, NotAnAggregateType) {
  // std::set<std::tuple<int>> xs;
  // lra::Iterable<std::set<std::tuple<int>>> i = lra::make_iterable(&xs);
  // lra::GroupBy<decltype(i), ra::Keys<0>, int> group_by =
  //     i | lra::group_by<ra::Keys<0>, int>();
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
