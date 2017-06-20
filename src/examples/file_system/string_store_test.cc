#include "examples/file_system/string_store.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

TEST(StringStore, ReadAndWrite) {
  StringStore ss;

  // Perform some initial writes.
  EXPECT_EQ("", ss.Read(0, 100));
  ss.Write(0, "ab");
  EXPECT_EQ("ab", ss.Read(0, 100));
  ss.Write(3, "cde");
  EXPECT_EQ("ab cde", ss.Read(0, 100));
  ss.Write(7, "fg");
  EXPECT_EQ("ab cde fg", ss.Read(0, 100));

  // Do some more complex reads.
  EXPECT_EQ("", ss.Read(0, 0));
  EXPECT_EQ("a", ss.Read(0, 1));
  EXPECT_EQ("b", ss.Read(1, 2));
  EXPECT_EQ(" ", ss.Read(2, 3));
  EXPECT_EQ("c", ss.Read(3, 4));
  EXPECT_EQ("d", ss.Read(4, 5));
  EXPECT_EQ("e", ss.Read(5, 6));
  EXPECT_EQ(" ", ss.Read(6, 7));
  EXPECT_EQ("f", ss.Read(7, 8));
  EXPECT_EQ("g", ss.Read(8, 9));
  EXPECT_EQ("ab", ss.Read(0, 2));
  EXPECT_EQ("cde", ss.Read(3, 6));
  EXPECT_EQ("fg", ss.Read(7, 9));
  EXPECT_EQ("b cde f", ss.Read(1, 8));

  // Overlap writes.
  ss.Write(1, "hijklmn");
  EXPECT_EQ("ahijklmng", ss.Read(0, 100));
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
