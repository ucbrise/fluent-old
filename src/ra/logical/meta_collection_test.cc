#include "ra/logical/meta_collection.h"

#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "collections/table.h"
#include "common/macros.h"

namespace lra = fluent::ra::logical;

namespace fluent {

TEST(MetaCollection, SimpleCompileCheck) {
  Table<int> t("t", {{"x"}});
  lra::MetaCollection<Table<int>> meta_collection =
      lra::make_meta_collection(&t);
  UNUSED(meta_collection);

  using actual = decltype(meta_collection)::column_types;
  using expected = TypeList<std::tuple<int>, LocalTupleId>;
  static_assert(StaticAssert<std::is_same<actual, expected>>::value, "");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
