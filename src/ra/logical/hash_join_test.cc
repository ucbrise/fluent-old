#include "ra/logical/hash_join.h"

#include <set>
#include <tuple>
#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "ra/logical/iterable.h"

namespace ra = fluent::ra;
namespace lra = fluent::ra::logical;

namespace fluent {

TEST(Map, SimpleCompileCheck) {
  std::set<std::tuple<int>> xs;
  std::set<std::tuple<int>> ys;
  lra::Iterable<std::set<std::tuple<int>>> ixs = lra::make_iterable(&xs);
  lra::Iterable<std::set<std::tuple<int>>> iys = lra::make_iterable(&ys);
  using leftks = ra::LeftKeys<0>;
  using rightks = ra::RightKeys<0>;
  using type = lra::HashJoin<decltype(ixs), leftks,  //
                             decltype(iys), rightks>;
  type hash_join = lra::make_hash_join<leftks, rightks>(ixs, iys);

  using actual = decltype(hash_join)::column_types;
  using expected = common::TypeList<int, int>;
  static_assert(common::StaticAssert<std::is_same<actual, expected>>::value,
                "");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
