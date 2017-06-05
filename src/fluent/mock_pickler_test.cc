#include "fluent/mock_pickler.h"

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace fluent {

using ::testing::UnorderedElementsAreArray;

TEST(MockPickler, PickleString) {
  EXPECT_EQ("", MockPickler<std::string>().Dump(""));
  EXPECT_EQ("", MockPickler<std::string>().Load(""));
  EXPECT_EQ("zardoz", MockPickler<std::string>().Dump("zardoz"));
  EXPECT_EQ("zardoz", MockPickler<std::string>().Load("zardoz"));
}

TEST(MockPickler, PickleChar) {
  EXPECT_EQ("a", MockPickler<char>().Dump('a'));
  EXPECT_EQ('a', MockPickler<char>().Load("a"));
}

TEST(MockPickler, PickleBool) {
  EXPECT_EQ("true", MockPickler<bool>().Dump(true));
  EXPECT_EQ(true, MockPickler<bool>().Load("true"));
  EXPECT_EQ("false", MockPickler<bool>().Dump(false));
  EXPECT_EQ(false, MockPickler<bool>().Load("false"));
}

TEST(MockPickler, PickleInt) {
  EXPECT_EQ("42", MockPickler<int>().Dump(42));
  EXPECT_EQ(42, MockPickler<int>().Load("42"));
}

TEST(MockPickler, PickleLong) {
  EXPECT_EQ("42", MockPickler<long>().Dump(42));
  EXPECT_EQ(42l, MockPickler<long>().Load("42"));
}

TEST(MockPickler, PickleLongLong) {
  EXPECT_EQ("42", MockPickler<long long>().Dump(42));
  EXPECT_EQ(42ll, MockPickler<long long>().Load("42"));
}

TEST(MockPickler, PickleUnsignedLong) {
  EXPECT_EQ("42", MockPickler<unsigned long>().Dump(42));
  EXPECT_EQ(42ul, MockPickler<unsigned long>().Load("42"));
}

TEST(MockPickler, PickleUnsignedLongLong) {
  EXPECT_EQ("42", MockPickler<unsigned long long>().Dump(42));
  EXPECT_EQ(42ull, MockPickler<unsigned long long>().Load("42"));
}

TEST(MockPickler, PickleFloat) {
  EXPECT_EQ("42.500000", MockPickler<float>().Dump(42.5));
  EXPECT_EQ(42.5, MockPickler<float>().Load("42.500000"));
}

TEST(MockPickler, PickleDouble) {
  EXPECT_EQ("42.500000", MockPickler<double>().Dump(42.5));
  EXPECT_EQ(42.5, MockPickler<double>().Load("42.500000"));
}

TEST(MockPickler, PickleLongDouble) {
  EXPECT_EQ("42.500000", MockPickler<long double>().Dump(42.5));
  EXPECT_EQ(42.5, MockPickler<long double>().Load("42.500000"));
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
