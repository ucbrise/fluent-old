#include "collections/stdout.h"

#include <cstddef>

#include <iostream>
#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "testing/captured_stdout.h"

namespace fluent {
namespace collections {

TEST(Stdout, Merge) {
  Stdout stdout_;
  testing::CapturedStdout captured;

  EXPECT_STREQ("", captured.Get().c_str());
  stdout_.Merge({"hello"}, 0x0, 0);
  EXPECT_STREQ("hello\n", captured.Get().c_str());
  stdout_.Merge({"world"}, 0x0, 0);
  EXPECT_STREQ("hello\nworld\n", captured.Get().c_str());
}

TEST(Table, DeferredMerge) {
  Stdout stdout_;
  testing::CapturedStdout captured;

  EXPECT_STREQ("", captured.Get().c_str());
  stdout_.DeferredMerge({"hello"}, 0x0, 0);
  EXPECT_STREQ("", captured.Get().c_str());
  stdout_.DeferredMerge({"world"}, 0x0, 0);
  EXPECT_STREQ("", captured.Get().c_str());
  stdout_.Tick();
  EXPECT_STREQ("hello\nworld\n", captured.Get().c_str());
}

}  // namespace collections
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
