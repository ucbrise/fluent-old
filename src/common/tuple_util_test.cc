#include "common/tuple_util.h"

#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(TupleUtil, AddTupleIter) {
  double sum = 0.0;
  TupleIter(std::tuple<int, float, double>{1, 2.0, 3.0},
            [&sum](auto x) { sum += x; });
  EXPECT_EQ(sum, 6.0);
}

TEST(TupleUtil, AddStringTupleIter) {
  std::string s = "";
  TupleIter(std::tuple<std::string, std::string>{"a", "b"},
            [&s](auto x) { s += x; });
  EXPECT_EQ(s, "ab");
}

TEST(TupleUtil, TupleIteri) {
  double sum = 0.0;
  TupleIteri(std::tuple<int, float, double>{1, 2.0, 3.0},
             [&sum](std::size_t i, auto x) { sum += i, sum += x; });
  EXPECT_EQ(sum, 9.0);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
