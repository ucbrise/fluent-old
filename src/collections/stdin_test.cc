#include "collections/stdin.h"

#include <cstddef>

#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace fluent {
namespace collections {

TEST(Stdin, StdinStartsEmpty) {
  Stdin stdin_;
  std::map<std::tuple<std::string>, CollectionTupleIds> expected;
  EXPECT_EQ(stdin_.Get(), expected);
}

TEST(Stdin, Merge) {
  Stdin stdin_;
  std::map<std::tuple<std::string>, CollectionTupleIds> expected;

  stdin_.Merge({"a"}, 0xA, 0);
  expected = {{{"a"}, {0xA, {0}}}};
  EXPECT_EQ(stdin_.Get(), expected);

  stdin_.Merge({"b"}, 0xB, 1);
  expected = {{{"a"}, {0xA, {0}}}, {{"b"}, {0xB, {1}}}};
  EXPECT_EQ(stdin_.Get(), expected);

  stdin_.Merge({"b"}, 0xB, 2);
  expected = {{{"a"}, {0xA, {0}}}, {{"b"}, {0xB, {1, 2}}}};
  EXPECT_EQ(stdin_.Get(), expected);
}

TEST(Stdin, TickClearsStdin) {
  Stdin stdin_;
  std::map<std::tuple<std::string>, CollectionTupleIds> expected;

  stdin_.Merge({"a"}, 0xA, 0);
  expected = {{{"a"}, {0xA, {0}}}};
  EXPECT_EQ(stdin_.Get(), expected);

  stdin_.Tick();
  expected = {};
  EXPECT_EQ(stdin_.Get(), expected);
}

}  // namespace collections
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
