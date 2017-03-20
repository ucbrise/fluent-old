#include "common/string_util.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(StringUtil, Join) {
  EXPECT_EQ(Join(), "");
  EXPECT_EQ(Join(1), "1");
  EXPECT_EQ(Join(1, 2), "1, 2");
  EXPECT_EQ(Join(1, 2, 3), "1, 2, 3");

  using namespace std::literals::string_literals;
  EXPECT_EQ(Join("a"s), "a"s);
  EXPECT_EQ(Join("a"s, "bc"s), "a, bc"s);
  EXPECT_EQ(Join("a"s, "bc"s, "def"s), "a, bc, def"s);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
