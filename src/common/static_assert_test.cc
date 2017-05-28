#include "common/static_assert.h"

#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(StaticAssert, Fail) {
  static_assert(StaticAssert<std::is_same<int, int>>::value, "");
}

}  // namespace fluent

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
