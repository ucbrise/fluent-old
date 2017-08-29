#include "ra/logical/collection.h"

#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "collections/table.h"
#include "common/macros.h"

namespace lra = fluent::ra::logical;

namespace fluent {

TEST(Collection, SimpleCompileCheck) {
  collections::Table<int> t("t", {{"x"}});
  lra::Collection<collections::Table<int>> collection =
      lra::make_collection(&t);
  UNUSED(collection);

  using actual = decltype(collection)::column_types;
  using expected = common::TypeList<int>;
  static_assert(common::StaticAssert<std::is_same<actual, expected>>::value,
                "");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
