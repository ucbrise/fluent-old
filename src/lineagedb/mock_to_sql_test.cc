#include "lineagedb/mock_to_sql.h"

#include <cstddef>
#include <cstdint>

#include <array>
#include <chrono>
#include <string>
#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "testing/mock_clock.h"

namespace fluent {
namespace lineagedb {

TEST(MockToSql, ToSqlType) {
  using time_point = std::chrono::time_point<testing::MockClock>;

  EXPECT_EQ(MockToSql<bool>().Type(), "bool");
  EXPECT_EQ(MockToSql<char>().Type(), "char");
  EXPECT_EQ(MockToSql<std::string>().Type(), "string");
  EXPECT_EQ(MockToSql<short int>().Type(), "short int");
  EXPECT_EQ(MockToSql<int>().Type(), "int");
  EXPECT_EQ(MockToSql<long>().Type(), "long");
  EXPECT_EQ(MockToSql<long long>().Type(), "long long");
  EXPECT_EQ(MockToSql<float>().Type(), "float");
  EXPECT_EQ(MockToSql<double>().Type(), "double");
  EXPECT_EQ(MockToSql<std::vector<int>>().Type(), "vector<int>");
  EXPECT_EQ((MockToSql<std::array<int, 2>>().Type()), "array<int, 2>");
  EXPECT_EQ(MockToSql<time_point>().Type(), "time_point");
}

TEST(MockToSql, ToSqlValue) {
  using time_point = std::chrono::time_point<testing::MockClock>;

  EXPECT_EQ(MockToSql<bool>().Value(true), "true");
  EXPECT_EQ(MockToSql<char>().Value('a'), "a");
  EXPECT_EQ(MockToSql<std::string>().Value("foo"), "foo");
  EXPECT_EQ(MockToSql<short int>().Value(1), "1");
  EXPECT_EQ(MockToSql<int>().Value(2), "2");
  EXPECT_EQ(MockToSql<long>().Value(3), "3");
  EXPECT_EQ(MockToSql<long long>().Value(4), "4");
  EXPECT_EQ(MockToSql<std::int64_t>().Value(5), "5");
  EXPECT_EQ(MockToSql<float>().Value(6.0), "6.000000");
  EXPECT_EQ(MockToSql<double>().Value(7.0), "7.000000");
  EXPECT_EQ((MockToSql<std::vector<int>>().Value({1, 2})), "[1, 2]");
  EXPECT_EQ((MockToSql<std::array<int, 2>>().Value({{1, 2}})), "[1, 2]");
  EXPECT_EQ(MockToSql<time_point>().Value(time_point(std::chrono::seconds(1))),
            "epoch + 1 seconds");
}

}  // namespace lineagedb
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
