#include "ra/logical/project.h"

#include <set>
#include <tuple>
#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/macros.h"
#include "ra/logical/iterable.h"

namespace lra = fluent::ra::logical;

namespace fluent {

TEST(Project, SimpleCompileCheck) {
  std::set<std::tuple<int, bool>> xs;
  lra::Iterable<std::set<std::tuple<int, bool>>> i = lra::make_iterable(&xs);
  lra::Project<decltype(i), 1, 0> project = i | lra::project<1, 0>();
  UNUSED(project);

  using actual = decltype(project)::column_types;
  using expected = TypeList<bool, int>;
  static_assert(StaticAssert<std::is_same<actual, expected>>::value, "");
}

// This code should NOT compile.
TEST(Project, ProjectionOutOfBounds) {
  // std::set<std::tuple<int, bool>> xs;
  // lra::Iterable<std::set<std::tuple<int, bool>>> i = lra::make_iterable(&xs);
  // lra::Project<decltype(i), 1, 0, 2> project = i | lra::project<1, 0, 2>();
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
