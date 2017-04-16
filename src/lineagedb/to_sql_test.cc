#include "lineagedb/to_sql.h"

#include <cstddef>
#include <cstdint>

#include <string>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace lineagedb {

template <typename T>
using Type = ToSqlType<ToSql>::type<T>;

template <typename T>
using Value = ToSqlValue<ToSql>::type<T>;

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
  EXPECT_EQ(ToSql<double>().Type(), "double precision");
  EXPECT_EQ(ToSql<double>().Type(), "double precision");
  EXPECT_EQ((ToSql<std::array<int, 0>>().Type()), "integer[]");
  EXPECT_EQ((ToSql<std::array<bool, 1>>().Type()), "boolean[]");

  EXPECT_EQ(Type<bool>()(), "boolean");
  EXPECT_EQ(Type<char>()(), "char(1)");
  EXPECT_EQ(Type<std::string>()(), "text");
  EXPECT_EQ(Type<short int>()(), "smallint");
  EXPECT_EQ(Type<int>()(), "integer");
  EXPECT_EQ(Type<long>()(), "bigint");
  EXPECT_EQ(Type<long long>()(), "bigint");
  EXPECT_EQ(Type<std::int64_t>()(), "bigint");
  EXPECT_EQ(Type<float>()(), "real");
  EXPECT_EQ(Type<double>()(), "double precision");
  EXPECT_EQ(Type<double>()(), "double precision");
  EXPECT_EQ(Type<double>()(), "double precision");
  EXPECT_EQ((Type<std::array<int, 0>>()()), "integer[]");
  EXPECT_EQ((Type<std::array<bool, 1>>()()), "boolean[]");

  // TODO(mwhittaker): Test ToSql<std::chrono::time_point<Clock>>.
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
  EXPECT_EQ((ToSql<std::array<int, 0>>().Value({{}})), "ARRAY[]");
  EXPECT_EQ((ToSql<std::array<bool, 2>>().Value({{true, false}})),
            "ARRAY[true, false]");

  EXPECT_EQ(Value<bool>()(true), "true");
  EXPECT_EQ(Value<char>()('a'), "'a'");
  EXPECT_EQ(Value<std::string>()("foo"), "'foo'");
  EXPECT_EQ(Value<short int>()(1), "1");
  EXPECT_EQ(Value<int>()(2), "2");
  EXPECT_EQ(Value<long>()(3), "3");
  EXPECT_EQ(Value<long long>()(4), "4");
  EXPECT_EQ(Value<std::int64_t>()(5), "5");
  EXPECT_EQ(Value<float>()(6.0), "6.000000");
  EXPECT_EQ(Value<double>()(7.0), "7.000000");
  EXPECT_EQ((Value<std::array<int, 0>>()({{}})), "ARRAY[]");
  EXPECT_EQ((Value<std::array<bool, 2>>()({{true, false}})),
            "ARRAY[true, false]");
  // TODO(mwhittaker): Test ToSqlValue<std::chrono::time_point<Clock>>.
}

}  // namespace lineagedb
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
