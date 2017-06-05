#include "common/tuple_util.h"

#include <sstream>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/status.h"
#include "common/status_macros.h"

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

TEST(TupleUtil, TupleIteriStatusConst) {
  int count = 0;
  auto f = [&count](std::size_t i, const auto& x) -> Status {
    Status s;
    if (x % 2 == 0) {
      s = Status::OK;
    } else {
      s = Status(ErrorCode::INVALID_ARGUMENT, std::to_string(i));
    }
    count += i;
    return s;
  };

  count = 0;
  EXPECT_EQ(TupleIteriStatus(std::tuple<>(), f), Status::OK);
  EXPECT_EQ(count, 0);

  count = 0;
  EXPECT_EQ(TupleIteriStatus(std::tuple<int>(0), f), Status::OK);
  EXPECT_EQ(count, 0);

  count = 0;
  EXPECT_EQ((TupleIteriStatus(std::tuple<int, int>(0, 2), f)), Status::OK);
  EXPECT_EQ(count, 1);

  count = 0;
  EXPECT_EQ((TupleIteriStatus(std::tuple<int, int, int>(0, 2, 4), f)),
            Status::OK);
  EXPECT_EQ(count, 3);

  count = 0;
  EXPECT_EQ((TupleIteriStatus(std::tuple<int, int, int>(0, 2, 5), f)),
            Status(ErrorCode::INVALID_ARGUMENT, "2"));
  EXPECT_EQ(count, 3);

  count = 0;
  EXPECT_EQ((TupleIteriStatus(std::tuple<int, int, int>(0, 3, 5), f)),
            Status(ErrorCode::INVALID_ARGUMENT, "1"));
  EXPECT_EQ(count, 1);

  count = 0;
  EXPECT_EQ((TupleIteriStatus(std::tuple<int, int, int>(1, 3, 5), f)),
            Status(ErrorCode::INVALID_ARGUMENT, "0"));
  EXPECT_EQ(count, 0);

  count = 0;
  EXPECT_EQ((TupleIteriStatus(std::tuple<int, int, int>(0, 3, 4), f)),
            Status(ErrorCode::INVALID_ARGUMENT, "1"));
  EXPECT_EQ(count, 1);
}

TEST(TupleUtil, TupleIteriStatusNonConst) {
  int count = 0;
  auto f = [&count](std::size_t i, auto& x) -> Status {
    Status s;
    if (x % 2 == 0) {
      s = Status::OK;
    } else {
      s = Status(ErrorCode::INVALID_ARGUMENT, std::to_string(i));
    }
    x++;
    count += i;
    return s;
  };

  {
    count = 0;
    std::tuple<> x;
    EXPECT_EQ(TupleIteriStatus(x, f), Status::OK);
    EXPECT_EQ(x, std::tuple<>());
    EXPECT_EQ(count, 0);
  }

  {
    count = 0;
    std::tuple<int> x(0);
    EXPECT_EQ(TupleIteriStatus(x, f), Status::OK);
    EXPECT_EQ(x, std::tuple<int>(1));
    EXPECT_EQ(count, 0);
  }

  {
    count = 0;
    std::tuple<int, int> x(0, 2);
    EXPECT_EQ(TupleIteriStatus(x, f), Status::OK);
    EXPECT_EQ(x, (std::tuple<int, int>(1, 3)));
    EXPECT_EQ(count, 1);
  }

  {
    count = 0;
    std::tuple<int, int, int> x(0, 2, 4);
    EXPECT_EQ(TupleIteriStatus(x, f), Status::OK);
    EXPECT_EQ(x, (std::tuple<int, int, int>(1, 3, 5)));
    EXPECT_EQ(count, 3);
  }

  {
    count = 0;
    std::tuple<int, int, int> x(0, 2, 5);
    EXPECT_EQ(TupleIteriStatus(x, f), Status(ErrorCode::INVALID_ARGUMENT, "2"));
    EXPECT_EQ(x, (std::tuple<int, int, int>(1, 3, 6)));
    EXPECT_EQ(count, 3);
  }

  {
    count = 0;
    std::tuple<int, int, int> x(0, 3, 5);
    EXPECT_EQ(TupleIteriStatus(x, f), Status(ErrorCode::INVALID_ARGUMENT, "1"));
    EXPECT_EQ(x, (std::tuple<int, int, int>(1, 4, 5)));
    EXPECT_EQ(count, 1);
  }

  {
    count = 0;
    std::tuple<int, int, int> x(1, 3, 5);
    EXPECT_EQ(TupleIteriStatus(x, f), Status(ErrorCode::INVALID_ARGUMENT, "0"));
    EXPECT_EQ(x, (std::tuple<int, int, int>(2, 3, 5)));
    EXPECT_EQ(count, 0);
  }

  {
    count = 0;
    std::tuple<int, int, int> x(0, 3, 4);
    EXPECT_EQ(TupleIteriStatus(x, f), Status(ErrorCode::INVALID_ARGUMENT, "1"));
    EXPECT_EQ(x, (std::tuple<int, int, int>(1, 4, 4)));
    EXPECT_EQ(count, 1);
  }
}

TEST(TupleUtil, TupleIterStatusConst) {
  int i = 0;
  auto f = [&i](const auto& x) -> Status {
    Status s;
    if (x % 2 == 0) {
      s = Status::OK;
    } else {
      s = Status(ErrorCode::INVALID_ARGUMENT, std::to_string(i));
    }
    i++;
    return s;
  };

  i = 0;
  EXPECT_EQ(TupleIterStatus(std::tuple<>(), f), Status::OK);
  EXPECT_EQ(i, 0);

  i = 0;
  EXPECT_EQ(TupleIterStatus(std::tuple<int>(0), f), Status::OK);
  EXPECT_EQ(i, 1);

  i = 0;
  EXPECT_EQ((TupleIterStatus(std::tuple<int, int>(0, 2), f)), Status::OK);
  EXPECT_EQ(i, 2);

  i = 0;
  EXPECT_EQ((TupleIterStatus(std::tuple<int, int, int>(0, 2, 4), f)),
            Status::OK);
  EXPECT_EQ(i, 3);

  i = 0;
  EXPECT_EQ((TupleIterStatus(std::tuple<int, int, int>(0, 2, 5), f)),
            Status(ErrorCode::INVALID_ARGUMENT, "2"));
  EXPECT_EQ(i, 3);

  i = 0;
  EXPECT_EQ((TupleIterStatus(std::tuple<int, int, int>(0, 3, 5), f)),
            Status(ErrorCode::INVALID_ARGUMENT, "1"));
  EXPECT_EQ(i, 2);

  i = 0;
  EXPECT_EQ((TupleIterStatus(std::tuple<int, int, int>(1, 3, 5), f)),
            Status(ErrorCode::INVALID_ARGUMENT, "0"));
  EXPECT_EQ(i, 1);

  i = 0;
  EXPECT_EQ((TupleIterStatus(std::tuple<int, int, int>(0, 3, 4), f)),
            Status(ErrorCode::INVALID_ARGUMENT, "1"));
  EXPECT_EQ(i, 2);
}

TEST(TupleUtil, TupleIterStatusNonConst) {
  int i = 0;
  auto f = [&i](auto& x) -> Status {
    Status s;
    if (x % 2 == 0) {
      s = Status::OK;
    } else {
      s = Status(ErrorCode::INVALID_ARGUMENT, std::to_string(i));
    }
    x++;
    i++;
    return s;
  };

  {
    i = 0;
    std::tuple<> x;
    EXPECT_EQ(TupleIterStatus(x, f), Status::OK);
    EXPECT_EQ(x, std::tuple<>());
    EXPECT_EQ(i, 0);
  }

  {
    i = 0;
    std::tuple<int> x(0);
    EXPECT_EQ(TupleIterStatus(x, f), Status::OK);
    EXPECT_EQ(x, std::tuple<int>(1));
    EXPECT_EQ(i, 1);
  }

  {
    i = 0;
    std::tuple<int, int> x(0, 2);
    EXPECT_EQ(TupleIterStatus(x, f), Status::OK);
    EXPECT_EQ(x, (std::tuple<int, int>(1, 3)));
    EXPECT_EQ(i, 2);
  }

  {
    i = 0;
    std::tuple<int, int, int> x(0, 2, 4);
    EXPECT_EQ(TupleIterStatus(x, f), Status::OK);
    EXPECT_EQ(x, (std::tuple<int, int, int>(1, 3, 5)));
    EXPECT_EQ(i, 3);
  }

  {
    i = 0;
    std::tuple<int, int, int> x(0, 2, 5);
    EXPECT_EQ(TupleIterStatus(x, f), Status(ErrorCode::INVALID_ARGUMENT, "2"));
    EXPECT_EQ(x, (std::tuple<int, int, int>(1, 3, 6)));
    EXPECT_EQ(i, 3);
  }

  {
    i = 0;
    std::tuple<int, int, int> x(0, 3, 5);
    EXPECT_EQ(TupleIterStatus(x, f), Status(ErrorCode::INVALID_ARGUMENT, "1"));
    EXPECT_EQ(x, (std::tuple<int, int, int>(1, 4, 5)));
    EXPECT_EQ(i, 2);
  }

  {
    i = 0;
    std::tuple<int, int, int> x(1, 3, 5);
    EXPECT_EQ(TupleIterStatus(x, f), Status(ErrorCode::INVALID_ARGUMENT, "0"));
    EXPECT_EQ(x, (std::tuple<int, int, int>(2, 3, 5)));
    EXPECT_EQ(i, 1);
  }

  {
    i = 0;
    std::tuple<int, int, int> x(0, 3, 4);
    EXPECT_EQ(TupleIterStatus(x, f), Status(ErrorCode::INVALID_ARGUMENT, "1"));
    EXPECT_EQ(x, (std::tuple<int, int, int>(1, 4, 4)));
    EXPECT_EQ(i, 2);
  }
}

TEST(TupleUtil, TupleToVector) {
  std::tuple<int, int, int> t{1, 2, 3};
  std::vector<int> actual = TupleToVector(t);
  std::vector<int> expected = {1, 2, 3};
  EXPECT_EQ(actual, expected);
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

TEST(TupleUtil, TupleProjectBySizetList) {
  std::tuple<int, char, float> t{0, '1', 2.0};

  EXPECT_EQ(TupleProjectBySizetList<SizetList<>>(t), std::tuple<>{});

  EXPECT_EQ(TupleProjectBySizetList<SizetList<0>>(t), std::tuple<int>{0});
  EXPECT_EQ(TupleProjectBySizetList<SizetList<1>>(t), std::tuple<char>{'1'});
  EXPECT_EQ(TupleProjectBySizetList<SizetList<2>>(t), std::tuple<char>{2.0});

  // Arguments are parenthesized so that the EXPECT_EQ macro doesn't get
  // confused about all the commas.
  EXPECT_EQ((TupleProjectBySizetList<SizetList<0, 0>>(t)),
            (std::tuple<int, int>{0, 0}));
  EXPECT_EQ((TupleProjectBySizetList<SizetList<0, 1>>(t)),
            (std::tuple<int, char>{0, '1'}));
  EXPECT_EQ((TupleProjectBySizetList<SizetList<0, 2>>(t)),
            (std::tuple<int, float>{0, 2.0}));
  EXPECT_EQ((TupleProjectBySizetList<SizetList<1, 0>>(t)),
            (std::tuple<char, int>{'1', 0}));
  EXPECT_EQ((TupleProjectBySizetList<SizetList<1, 1>>(t)),
            (std::tuple<char, char>{'1', '1'}));
  EXPECT_EQ((TupleProjectBySizetList<SizetList<1, 2>>(t)),
            (std::tuple<char, float>{'1', 2.0}));
  EXPECT_EQ((TupleProjectBySizetList<SizetList<2, 0>>(t)),
            (std::tuple<float, int>{2.0, 0}));
  EXPECT_EQ((TupleProjectBySizetList<SizetList<2, 1>>(t)),
            (std::tuple<float, char>{2.0, '1'}));
  EXPECT_EQ((TupleProjectBySizetList<SizetList<2, 2>>(t)),
            (std::tuple<float, float>{2.0, 2.0}));

  auto actual = TupleProjectBySizetList<SizetList<0, 1, 0, 2, 1>>(t);
  auto expected = std::tuple<int, char, int, float, char>{0, '1', 0, 2.0, '1'};
  EXPECT_EQ(actual, expected);
}

TEST(TupleUtil, TupleTake) {
  std::tuple<int, char, float, bool> t = {0, '1', 2.0, false};
  std::tuple<> t0 = {};
  std::tuple<int> t1 = {0};
  std::tuple<int, char> t2 = {0, '1'};
  std::tuple<int, char, float> t3 = {0, '1', 2.0};
  std::tuple<int, char, float, bool> t4 = {0, '1', 2.0, false};

  EXPECT_EQ(TupleTake<0>(t), t0);
  EXPECT_EQ(TupleTake<1>(t), t1);
  EXPECT_EQ(TupleTake<2>(t), t2);
  EXPECT_EQ(TupleTake<3>(t), t3);
  EXPECT_EQ(TupleTake<4>(t), t4);
  EXPECT_EQ(TupleTake<5>(t), t4);
}

TEST(TupleUtil, TupleDrop) {
  std::tuple<int, char, float, bool> t = {0, '1', 2.0, false};
  std::tuple<int, char, float, bool> t0 = {0, '1', 2.0, false};
  std::tuple<char, float, bool> t1 = {'1', 2.0, false};
  std::tuple<float, bool> t2 = {2.0, false};
  std::tuple<bool> t3 = {false};
  std::tuple<> t4 = {};

  EXPECT_EQ(TupleDrop<0>(t), t0);
  EXPECT_EQ(TupleDrop<1>(t), t1);
  EXPECT_EQ(TupleDrop<2>(t), t2);
  EXPECT_EQ(TupleDrop<3>(t), t3);
  EXPECT_EQ(TupleDrop<4>(t), t4);
  EXPECT_EQ(TupleDrop<5>(t), t4);
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
