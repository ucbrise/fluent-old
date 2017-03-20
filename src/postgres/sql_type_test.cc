#include "postgres/sql_type.h"

#include <cstddef>
#include <cstdint>

#include <string>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace postgres {

TEST(SqlType, SqlTypeType) {
  EXPECT_EQ(SqlType<bool>().type(), "boolean");
  EXPECT_EQ(SqlType<char>().type(), "char(1)");
  EXPECT_EQ(SqlType<std::string>().type(), "text");
  EXPECT_EQ(SqlType<short int>().type(), "smallint");
  EXPECT_EQ(SqlType<int>().type(), "integer");
  EXPECT_EQ(SqlType<long>().type(), "bigint");
  EXPECT_EQ(SqlType<long long>().type(), "bigint");
  EXPECT_EQ(SqlType<std::int64_t>().type(), "bigint");
  EXPECT_EQ(SqlType<float>().type(), "real");
  EXPECT_EQ(SqlType<double>().type(), "double precision");
}

TEST(SqlType, SqlTypeValue) {
  EXPECT_EQ(SqlType<bool>().value(true), "true");
  EXPECT_EQ(SqlType<char>().value('a'), "'a'");
  EXPECT_EQ(SqlType<std::string>().value("foo"), "'foo'");
  EXPECT_EQ(SqlType<short int>().value(1), "1");
  EXPECT_EQ(SqlType<int>().value(2), "2");
  EXPECT_EQ(SqlType<long>().value(3), "3");
  EXPECT_EQ(SqlType<long long>().value(4), "4");
  EXPECT_EQ(SqlType<std::int64_t>().value(5), "5");
  EXPECT_EQ(SqlType<float>().value(6.0), "6.000000");
  EXPECT_EQ(SqlType<double>().value(7.0), "7.000000");
}

}  // namespace postgres
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
