#include "collections/table.h"

#include <cstddef>

#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(Table, TableStartsEmpty) {
  Table<char, char> t("t", {{"x", "y"}});
  std::map<std::tuple<char, char>, CollectionTupleIds> expected;
  EXPECT_EQ(t.Get(), expected);
}

TEST(Table, Merge) {
  Table<char, char> t("t", {{"x", "y"}});
  std::map<std::tuple<char, char>, CollectionTupleIds> expected;

  t.Merge({'a', 'a'}, 0xA, 0);
  expected = {{{'a', 'a'}, {0xA, {0}}}};
  EXPECT_EQ(t.Get(), expected);

  t.Merge({'b', 'b'}, 0xB, 1);
  expected = {{{'a', 'a'}, {0xA, {0}}}, {{'b', 'b'}, {0xB, {1}}}};
  EXPECT_EQ(t.Get(), expected);

  t.Merge({'b', 'b'}, 0xB, 2);
  expected = {{{'a', 'a'}, {0xA, {0}}}, {{'b', 'b'}, {0xB, {1, 2}}}};
  EXPECT_EQ(t.Get(), expected);
}

TEST(Table, TickDoesntClearTable) {
  Table<char, char> t("t", {{"x", "y"}});
  std::map<std::tuple<char, char>, CollectionTupleIds> expected;

  t.Merge({'a', 'a'}, 0xA, 0);
  expected = {{{'a', 'a'}, {0xA, {0}}}};
  EXPECT_EQ(t.Get(), expected);
  t.Tick();
  EXPECT_EQ(t.Get(), expected);
}

TEST(Table, DeferredMerge) {
  Table<char, char> t("t", {{"x", "y"}});
  std::map<std::tuple<char, char>, CollectionTupleIds> expected;

  t.DeferredMerge({'a', 'a'}, 0xA, 0);
  expected = {};
  EXPECT_EQ(t.Get(), expected);

  t.DeferredMerge({'b', 'b'}, 0xB, 1);
  expected = {};
  EXPECT_EQ(t.Get(), expected);

  t.DeferredMerge({'b', 'b'}, 0xB, 2);
  expected = {};
  EXPECT_EQ(t.Get(), expected);

  t.Tick();
  expected = {{{'a', 'a'}, {0xA, {0}}}, {{'b', 'b'}, {0xB, {1, 2}}}};
  EXPECT_EQ(t.Get(), expected);
}

TEST(Table, DeferredDelete) {
  Table<char, char> t("t", {{"x", "y"}});
  std::map<std::tuple<char, char>, CollectionTupleIds> expected;

  t.Merge({'a', 'a'}, 0xA, 0);
  t.Merge({'b', 'b'}, 0xB, 1);
  t.Merge({'b', 'b'}, 0xB, 2);
  expected = {{{'a', 'a'}, {0xA, {0}}}, {{'b', 'b'}, {0xB, {1, 2}}}};
  EXPECT_EQ(t.Get(), expected);

  t.DeferredDelete({'a', 'a'}, 0xA, 3);
  expected = {{{'a', 'a'}, {0xA, {0}}}, {{'b', 'b'}, {0xB, {1, 2}}}};
  EXPECT_EQ(t.Get(), expected);
  t.Tick();
  expected = {{{'b', 'b'}, {0xB, {1, 2}}}};
  EXPECT_EQ(t.Get(), expected);

  t.DeferredDelete({'c', 'c'}, 0xC, 4);
  expected = {{{'b', 'b'}, {0xB, {1, 2}}}};
  EXPECT_EQ(t.Get(), expected);
  t.Tick();
  EXPECT_EQ(t.Get(), expected);

  t.DeferredDelete({'b', 'b'}, 0xB, 5);
  expected = {{{'b', 'b'}, {0xB, {1, 2}}}};
  EXPECT_EQ(t.Get(), expected);
  t.Tick();
  expected = {};
  EXPECT_EQ(t.Get(), expected);
}

TEST(Table, DeferredMergeAndDeferredDelete) {
  Table<char, char> t("t", {{"x", "y"}});
  std::map<std::tuple<char, char>, CollectionTupleIds> expected;

  t.DeferredMerge({'a', 'a'}, 0xA, 0);
  t.DeferredMerge({'b', 'b'}, 0xB, 1);
  t.DeferredDelete({'b', 'b'}, 0xB, 2);
  t.DeferredDelete({'c', 'c'}, 0xC, 3);
  t.Tick();
  expected = {{{'a', 'a'}, {0xA, {0}}}};
  EXPECT_EQ(t.Get(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
