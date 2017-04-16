#include "common/tuple_util.h"

#include <sstream>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace detail {

template <typename T>
struct ToString;

template <>
struct ToString<int> {
  std::string operator()() { return "int"; }
};

template <>
struct ToString<bool> {
  std::string operator()() { return "bool"; }
};

template <>
struct ToString<char> {
  std::string operator()() { return "char"; }
};

template <typename T>
struct ToPointer;

template <>
struct ToPointer<int> {
  int* operator()() { return nullptr; }
};

template <>
struct ToPointer<bool> {
  bool* operator()() { return nullptr; }
};

template <>
struct ToPointer<char> {
  char* operator()() { return nullptr; }
};

}  // namespace detail

TEST(TupleUtil, TupleIter) {
  {
    double sum = 0.0;
    TupleIter(std::tuple<int, float, double>{1, 2.0, 3.0},
              [&sum](auto x) { sum += x; });
    EXPECT_EQ(sum, 6.0);
  }

  {
    double sum = 0.0;
    std::tuple<int, float, double> t{1, 2.0, 3.0};
    std::tuple<int, float, double> expected{2, 3.0, 4.0};
    TupleIter(t, [&sum](auto& x) {
      sum += x;
      x++;
    });
    EXPECT_EQ(sum, 6.0);
    EXPECT_EQ(t, expected);
  }

  {
    std::string s = "";
    TupleIter(std::tuple<std::string, std::string>{"a", "b"},
              [&s](auto x) { s += x; });
    EXPECT_EQ(s, "ab");
  }
}

TEST(TupleUtil, TupleIteri) {
  {
    double sum = 0.0;
    TupleIteri(std::tuple<int, float, double>{1, 2.0, 3.0},
               [&sum](std::size_t i, auto x) {
                 sum += i;
                 sum += x;
               });
    EXPECT_EQ(sum, 9.0);
  }

  {
    double sum = 0.0;
    std::tuple<int, float, double> t{1, 2.0, 3.0};
    std::tuple<int, float, double> expected{2, 3.0, 4.0};
    TupleIteri(t, [&sum](std::size_t i, auto& x) {
      sum += i;
      sum += x;
      x++;
    });
    EXPECT_EQ(sum, 9.0);
    EXPECT_EQ(t, expected);
  }
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

TEST(TupleUtil, TupleToVector) {
  std::tuple<int, int, int> t{1, 2, 3};
  std::vector<int> actual = TupleToVector(t);
  std::vector<int> expected = {1, 2, 3};
  EXPECT_EQ(actual, expected);
}

TEST(TupleUtil, TupleFromTypes) {
  {
    using tuple_t = std::tuple<std::string, std::string, std::string>;
    tuple_t actual = TupleFromTypes<detail::ToString, int, bool, char>();
    tuple_t expected = {"int", "bool", "char"};
    EXPECT_EQ(actual, expected);
  }

  {
    using tuple_t = std::tuple<int*, bool*, char*>;
    tuple_t actual = TupleFromTypes<detail::ToPointer, int, bool, char>();
    tuple_t expected = {nullptr, nullptr, nullptr};
    EXPECT_EQ(actual, expected);
  }
}

TEST(TupleUtil, TupleProject) {
  std::tuple<int, char, float> t{0, '1', 2.0};

  EXPECT_EQ(TupleProject<>(t), std::tuple<>{});

  EXPECT_EQ(TupleProject<0>(t), std::tuple<int>{0});
  EXPECT_EQ(TupleProject<1>(t), std::tuple<char>{'1'});
  EXPECT_EQ(TupleProject<2>(t), std::tuple<char>{2.0});

  // Arguments are parenthesized so that the EXPECT_EQ macro doesn't get
  // confused about all the commas.
  EXPECT_EQ((TupleProject<0, 0>(t)), (std::tuple<int, int>{0, 0}));
  EXPECT_EQ((TupleProject<0, 1>(t)), (std::tuple<int, char>{0, '1'}));
  EXPECT_EQ((TupleProject<0, 2>(t)), (std::tuple<int, float>{0, 2.0}));
  EXPECT_EQ((TupleProject<1, 0>(t)), (std::tuple<char, int>{'1', 0}));
  EXPECT_EQ((TupleProject<1, 1>(t)), (std::tuple<char, char>{'1', '1'}));
  EXPECT_EQ((TupleProject<1, 2>(t)), (std::tuple<char, float>{'1', 2.0}));
  EXPECT_EQ((TupleProject<2, 0>(t)), (std::tuple<float, int>{2.0, 0}));
  EXPECT_EQ((TupleProject<2, 1>(t)), (std::tuple<float, char>{2.0, '1'}));
  EXPECT_EQ((TupleProject<2, 2>(t)), (std::tuple<float, float>{2.0, 2.0}));

  auto actual = TupleProject<0, 1, 0, 2, 1>(t);
  auto expected = std::tuple<int, char, int, float, char>{0, '1', 0, 2.0, '1'};
  EXPECT_EQ(actual, expected);
}

TEST(TupleUtil, OstreamOperator) {
  std::tuple<int, char, float> t{1, '2', 3.0};
  std::ostringstream os;
  os << t;
  EXPECT_EQ(os.str(), "(1, 2, 3)");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}