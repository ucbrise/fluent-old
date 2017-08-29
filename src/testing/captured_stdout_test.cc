#include "testing/captured_stdout.h"

#include <iostream>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace testing {

TEST(CapturedStdout, SimpleTest) {
  std::cout << "fee" << std::endl;
  {
    CapturedStdout captured;
    std::cout << "fi" << std::endl;
    EXPECT_STREQ("fi\n", captured.Get().c_str());
    std::cout << "fo" << std::endl;
    EXPECT_STREQ("fi\nfo\n", captured.Get().c_str());
  }
  std::cout << "fum" << std::endl;
}

}  // namespace testing
}  // namespace fluent

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
