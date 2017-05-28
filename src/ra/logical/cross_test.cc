#include "ra/logical/cross.h"

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
  std::set<std::tuple<bool>> ys;
  lra::Iterable<std::set<std::tuple<int>>> ixs = lra::make_iterable(&xs);
  lra::Iterable<std::set<std::tuple<bool>>> iys = lra::make_iterable(&ys);
  lra::Cross<decltype(ixs), decltype(iys)> cross = lra::make_cross(ixs, iys);
  UNUSED(cross);

  using actual = decltype(cross)::column_types;
  using expected = TypeList<int, bool>;
  static_assert(StaticAssert<std::is_same<actual, expected>>::value, "");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
