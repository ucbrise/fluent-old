#include "common/cereal_pickler.h"

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace fluent {

using ::testing::UnorderedElementsAreArray;

TEST(CerealPickler, PickleString) {
  CerealPickler<std::string> pickler;
  EXPECT_EQ("", pickler.Load(pickler.Dump("")));
  EXPECT_EQ("foo", pickler.Load(pickler.Dump("foo")));
}

TEST(CerealPickler, PickleChar) {
  CerealPickler<char> pickler;
  EXPECT_EQ('a', pickler.Load(pickler.Dump('a')));
}

TEST(CerealPickler, PickleBool) {
  CerealPickler<bool> pickler;
  EXPECT_EQ(true, pickler.Load(pickler.Dump(true)));
  EXPECT_EQ(false, pickler.Load(pickler.Dump(false)));
}

TEST(CerealPickler, PickleInt) {
  CerealPickler<int> pickler;
  EXPECT_EQ(42, pickler.Load(pickler.Dump(42)));
}

TEST(CerealPickler, PickleLong) {
  CerealPickler<long> pickler;
  EXPECT_EQ(42l, pickler.Load(pickler.Dump(42l)));
}

TEST(CerealPickler, PickleLongLong) {
  CerealPickler<long long> pickler;
  EXPECT_EQ(42ll, pickler.Load(pickler.Dump(42ll)));
}

TEST(CerealPickler, PickleUnsignedLong) {
  CerealPickler<unsigned long> pickler;
  EXPECT_EQ(42ul, pickler.Load(pickler.Dump(42ul)));
}

TEST(CerealPickler, PickleUnsignedLongLong) {
  CerealPickler<unsigned long long> pickler;
  EXPECT_EQ(42ull, pickler.Load(pickler.Dump(42ull)));
}

TEST(CerealPickler, PickleFloat) {
  CerealPickler<float> pickler;
  EXPECT_EQ(42.5, pickler.Load(pickler.Dump(42.5)));
}

TEST(CerealPickler, PickleDouble) {
  CerealPickler<double> pickler;
  EXPECT_EQ(42.5, pickler.Load(pickler.Dump(42.5)));
}

TEST(CerealPickler, PickleLongDouble) {
  CerealPickler<long double> pickler;
  EXPECT_EQ(42.5, pickler.Load(pickler.Dump(42.5)));
}

TEST(CerealPickler, PickleVector) {
  CerealPickler<std::vector<int>> pickler;
  std::vector<int> xs = {1, 2, 3};
  EXPECT_EQ(xs, pickler.Load(pickler.Dump(xs)));
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
