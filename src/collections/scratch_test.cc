#include "collections/scratch.h"

#include <cstddef>

#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(Scratch, ScratchStartsEmpty) {
  Scratch<char, char> s("s", {{"x", "y"}});
  std::map<std::tuple<char, char>, CollectionTupleIds> expected;
  EXPECT_EQ(s.Get(), expected);
}

TEST(Scratch, Merge) {
  Scratch<char, char> s("s", {{"x", "y"}});
  std::map<std::tuple<char, char>, CollectionTupleIds> expected;

  s.Merge({'a', 'a'}, 0xA, 0);
  expected = {{{'a', 'a'}, {0xA, {0}}}};
  EXPECT_EQ(s.Get(), expected);

  s.Merge({'b', 'b'}, 0xB, 1);
  expected = {{{'a', 'a'}, {0xA, {0}}}, {{'b', 'b'}, {0xB, {1}}}};
  EXPECT_EQ(s.Get(), expected);

  s.Merge({'b', 'b'}, 0xB, 2);
  expected = {{{'a', 'a'}, {0xA, {0}}}, {{'b', 'b'}, {0xB, {1, 2}}}};
  EXPECT_EQ(s.Get(), expected);
}

TEST(Scratch, TickClearsScratch) {
  Scratch<char, char> s("s", {{"x", "y"}});
  std::map<std::tuple<char, char>, CollectionTupleIds> expected;

  s.Merge({'a', 'a'}, 0xA, 0);
  expected = {{{'a', 'a'}, {0xA, {0}}}};
  EXPECT_EQ(s.Get(), expected);

  s.Tick();
  expected = {};
  EXPECT_EQ(s.Get(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
