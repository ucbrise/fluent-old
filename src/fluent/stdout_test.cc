#include "fluent/stdout.h"

#include <iostream>
#include <set>
#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ra/all.h"
#include "testing/captured_stdout.h"

namespace fluent {

TEST(Stdout, SimpleMerge) {
  Stdout stdout;
  CapturedStdout captured;
  std::set<std::tuple<std::string>> ss = {{"hello"}, {"there"}};

  EXPECT_STREQ("", captured.Get().c_str());
  stdout.Merge(ss);
  EXPECT_STREQ("hello\nthere\n", captured.Get().c_str());
  stdout.Tick();
  EXPECT_STREQ("hello\nthere\n", captured.Get().c_str());
}

TEST(Table, SimpleDeferredMerge) {
  Stdout stdout;
  CapturedStdout captured;
  std::set<std::tuple<std::string>> ss = {{"hello"}, {"there"}};

  EXPECT_STREQ("", captured.Get().c_str());
  stdout.DeferredMerge(ss);
  EXPECT_STREQ("", captured.Get().c_str());
  stdout.Tick();
  EXPECT_STREQ("hello\nthere\n", captured.Get().c_str());
  stdout.Tick();
  EXPECT_STREQ("hello\nthere\n", captured.Get().c_str());
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
