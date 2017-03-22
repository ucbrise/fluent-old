#include "postgres/to_sql.h"

#include <cstddef>
#include <cstdint>

#include <string>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace postgres {

TEST(ToSql, ToSqlType) {
  EXPECT_EQ(ToSql<bool>().Type(), "boolean");
  EXPECT_EQ(ToSql<char>().Type(), "char(1)");
  EXPECT_EQ(ToSql<std::string>().Type(), "text");
  EXPECT_EQ(ToSql<short int>().Type(), "smallint");
  EXPECT_EQ(ToSql<int>().Type(), "integer");
  EXPECT_EQ(ToSql<long>().Type(), "bigint");
  EXPECT_EQ(ToSql<long long>().Type(), "bigint");
  EXPECT_EQ(ToSql<std::int64_t>().Type(), "bigint");
  EXPECT_EQ(ToSql<float>().Type(), "real");
  EXPECT_EQ(ToSql<double>().Type(), "double precision");
}

TEST(ToSql, ToSqlValue) {
  EXPECT_EQ(ToSql<bool>().Value(true), "true");
  EXPECT_EQ(ToSql<char>().Value('a'), "'a'");
  EXPECT_EQ(ToSql<std::string>().Value("foo"), "'foo'");
  EXPECT_EQ(ToSql<short int>().Value(1), "1");
  EXPECT_EQ(ToSql<int>().Value(2), "2");
  EXPECT_EQ(ToSql<long>().Value(3), "3");
  EXPECT_EQ(ToSql<long long>().Value(4), "4");
  EXPECT_EQ(ToSql<std::int64_t>().Value(5), "5");
  EXPECT_EQ(ToSql<float>().Value(6.0), "6.000000");
  EXPECT_EQ(ToSql<double>().Value(7.0), "7.000000");
}

}  // namespace postgres
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
