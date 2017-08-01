#include "common/string_util.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace common {

TEST(StringUtil, Join) {
  EXPECT_EQ(Join(), "");
  EXPECT_EQ(Join(1), "1");
  EXPECT_EQ(Join(1, 2), "1, 2");
  EXPECT_EQ(Join(1, 2, 3), "1, 2, 3");

  using namespace std::literals::string_literals;
  EXPECT_EQ(Join("a"s), "a"s);
  EXPECT_EQ(Join("a"s, "bc"s), "a, bc"s);
  EXPECT_EQ(Join("a"s, "bc"s, "def"s), "a, bc, def"s);

  EXPECT_EQ(Join(std::vector<std::string>{}), ""s);
  EXPECT_EQ(Join(std::vector<std::string>{"a"}), "a"s);
  EXPECT_EQ(Join(std::vector<std::string>{"a", "b"}), "a, b"s);
  EXPECT_EQ(Join(std::vector<std::string>{"a", "b", "c"}), "a, b, c"s);

  EXPECT_EQ(Join(std::array<std::string, 0>{}), ""s);
  EXPECT_EQ(Join(std::array<std::string, 1>{{"a"}}), "a"s);
  EXPECT_EQ(Join(std::array<std::string, 2>{{"a", "b"}}), "a, b"s);
  EXPECT_EQ(Join(std::array<std::string, 3>{{"a", "b", "c"}}), "a, b, c"s);
}

TEST(StringUtil, Split) {
  using stringvec = std::vector<std::string>;
  EXPECT_EQ((stringvec{}), Split(""));
  EXPECT_EQ((stringvec{"a"}), Split("a"));
  EXPECT_EQ((stringvec{"foo"}), Split("foo"));
  EXPECT_EQ((stringvec{"foo"}), Split(" foo"));
  EXPECT_EQ((stringvec{"foo"}), Split("foo "));
  EXPECT_EQ((stringvec{"foo"}), Split(" foo "));
  EXPECT_EQ((stringvec{"foo"}), Split("  foo  "));
  EXPECT_EQ((stringvec{"foo", "bar"}), Split("foo bar"));
  EXPECT_EQ((stringvec{"foo", "bar"}), Split("foo  bar"));
  EXPECT_EQ((stringvec{"foo", "bar"}), Split("foo  bar "));
  EXPECT_EQ((stringvec{"foo", "bar"}), Split(" foo  bar"));
  EXPECT_EQ((stringvec{"foo", "bar"}), Split(" foo  bar "));
  EXPECT_EQ((stringvec{"foo", "bar", "baz"}), Split("   foo   bar  baz "));
}

TEST(StringUtil, ToLower) {
  EXPECT_EQ(ToLower("foo"), "foo");
  EXPECT_EQ(ToLower("Foo"), "foo");
  EXPECT_EQ(ToLower("FOo"), "foo");
  EXPECT_EQ(ToLower("FOO"), "foo");
  EXPECT_EQ(ToLower("fOO"), "foo");
  EXPECT_EQ(ToLower("foO"), "foo");
}

TEST(StringUtil, ToUpper) {
  EXPECT_EQ(ToUpper("foo"), "FOO");
  EXPECT_EQ(ToUpper("Foo"), "FOO");
  EXPECT_EQ(ToUpper("FOo"), "FOO");
  EXPECT_EQ(ToUpper("FOO"), "FOO");
  EXPECT_EQ(ToUpper("fOO"), "FOO");
  EXPECT_EQ(ToUpper("foO"), "FOO");
}

TEST(StringUtil, CrunchWhitespace) {
  using namespace std::literals::string_literals;
  EXPECT_EQ(CrunchWhitespace(""), ""s);
  EXPECT_EQ(CrunchWhitespace(" "), " "s);
  EXPECT_EQ(CrunchWhitespace("  "), " "s);
  EXPECT_EQ(CrunchWhitespace("   "), " "s);
  EXPECT_EQ(CrunchWhitespace("a"), "a"s);
  EXPECT_EQ(CrunchWhitespace(" a"), " a"s);
  EXPECT_EQ(CrunchWhitespace(" a "), " a "s);
  EXPECT_EQ(CrunchWhitespace("  a "), " a "s);
  EXPECT_EQ(CrunchWhitespace("   a "), " a "s);
  EXPECT_EQ(CrunchWhitespace(" a  "), " a "s);
  EXPECT_EQ(CrunchWhitespace("  a  "), " a "s);
  EXPECT_EQ(CrunchWhitespace("   a  "), " a "s);
  EXPECT_EQ(CrunchWhitespace("\n"), " "s);
  EXPECT_EQ(CrunchWhitespace("\n\n"), " "s);
  EXPECT_EQ(CrunchWhitespace("\n \n"), " "s);
}

}  // namespace common
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
