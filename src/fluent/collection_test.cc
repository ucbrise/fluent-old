#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "fluent/table.h"
#include "ra/all.h"

using ::testing::UnorderedElementsAreArray;

namespace fluent {

TEST(Table, Name) {
  Table<int, int, int> foo("foo");
  EXPECT_EQ(foo.Name(), "foo");
  Table<int, int, int> bar("bar");
  EXPECT_EQ(bar.Name(), "bar");
}

TEST(Table, Get) {
  Table<int, int, int> foo("foo");
  std::set<std::tuple<int, int, int>> expected;
  EXPECT_THAT(foo.Get(), testing::UnorderedElementsAreArray(expected));
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
