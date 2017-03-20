#include "common/tuple_util.h"

#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(TupleUtil, TupleIter) {
  {
    double sum = 0.0;
    TupleIter(std::tuple<int, float, double>{1, 2.0, 3.0},
              [&sum](auto x) { sum += x; });
    EXPECT_EQ(sum, 6.0);
  }

  {
    std::string s = "";
    TupleIter(std::tuple<std::string, std::string>{"a", "b"},
              [&s](auto x) { s += x; });
    EXPECT_EQ(s, "ab");
  }
}

TEST(TupleUtil, TupleIteri) {
  double sum = 0.0;
  TupleIteri(std::tuple<int, float, double>{1, 2.0, 3.0},
             [&sum](std::size_t i, auto x) { sum += i, sum += x; });
  EXPECT_EQ(sum, 9.0);
}

TEST(TupleUtil, TupleMap) {
  {
    std::tuple<int, char, float> t = {1, '2', 3.0};
    auto result = TupleMap(t, [](const auto& x) { return x; });
    EXPECT_EQ(result, t);
  }

  {
    std::tuple<int, long, unsigned long> t = {1, 2l, 3ul};
    auto result = TupleMap(t, [](const auto& x) { return std::to_string(x); });
    std::tuple<std::string, std::string, std::string> expected = {"1", "2",
                                                                  "3"};
    EXPECT_EQ(result, expected);
  }
}

TEST(TupleUtil, TupleFold) {
  {
    std::tuple<short, int, long> t = {1, 2, 3};
    long actual = TupleFold(
        0l, t, [](const long& acc, const auto& x) { return acc + x; });
    EXPECT_EQ(actual, 6l);
  }

  {
    std::tuple<int, long, unsigned long> t = {1, 2l, 3ul};
    std::string acc = "";
    std::string actual =
        TupleFold(acc, t, [](const std::string& acc, const auto& x) {
          return acc + std::to_string(x);
        });
    std::string expected = "123";
    EXPECT_EQ(actual, expected);
  }
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
