#ifndef LINEAGEDB_TO_SQL_H_
#define LINEAGEDB_TO_SQL_H_

#include <cstdint>

#include <array>
#include <chrono>
#include <string>
#include <type_traits>
#include <vector>

#include "fmt/format.h"

#include "common/string_util.h"

namespace fluent {
namespace lineagedb {

// TODO(mwhittaker): Escape strings. This is actually pretty annoying to do
// because pqxx's functions to escape strings require a database connection.

// C++ code has a certain set of types and a certain set of values of each
// type. For example, the value `42` is a C++ `int`. Postgres has a certain
// set of types and a certain set of values of each type. For example, the
// value `42` is a lineagedb `integer`.
//
// The `ToSql` struct template provides a mechanism to convert C++ types to
// lineagedb types and C++ values to lineagedb values. More specifically,
// `ToSql<T>.Type()` will return the name of the lineagedb type corresponding to
// the C++ type `T`, and `ToSql<T>.Value(x)` will convert the C++ value `x`
// into a string which can be used to insert the value into lineagedb.
//
// For example, `ToSql<std::string>.Type()` is `"text"` and
// `ToSql<std::string>.Value("hello")` is `'hello'`.
template <typename T>
struct ToSql;

// ToSqlType<ToSql>::type<T>()() == ToSql<T>().Type().
template <template <typename> class ToSql>
struct ToSqlType {
  template <typename T>
  struct type {
    auto operator()() { return ToSql<T>().Type(); }
  };
};

// ToSqlValue<ToSql>::type<T>()(x) == ToSql<T>().value(x).
template <template <typename> class ToSql>
struct ToSqlValue {
  template <typename T>
  struct type {
    auto operator()(const T& x) { return ToSql<T>().Value(x); }
  };
};

template <>
struct ToSql<bool> {
  std::string Type() { return "boolean"; }
  std::string Value(bool b) { return b ? "true" : "false"; }
};

template <>
struct ToSql<char> {
  std::string Type() { return "char(1)"; }
  std::string Value(char c) { return fmt::format("'{}'", c); }
};

template <>
struct ToSql<std::string> {
  std::string Type() { return "text"; }
  std::string Value(const std::string& s) { return fmt::format("'{}'", s); }
};

template <>
struct ToSql<short int> {
  static_assert(sizeof(short int) == 2, "We assume short ints are 2 bytes.");
  std::string Type() { return "smallint"; }
  std::string Value(short int x) { return std::to_string(x); }
};

template <>
struct ToSql<int> {
  static_assert(sizeof(int) == 4, "We assume ints are 4 bytes.");
  std::string Type() { return "integer"; }
  std::string Value(int x) { return std::to_string(x); }
};

template <>
struct ToSql<long> {
  static_assert(sizeof(long) == 8, "We assume longs are 8 bytes.");
  std::string Type() { return "bigint"; }
  std::string Value(long x) { return std::to_string(x); }
};

template <>
struct ToSql<long long> {
  static_assert(sizeof(long long) == 8, "We assume long longs are 8 bytes.");
  std::string Type() { return "bigint"; }
  std::string Value(long long x) { return std::to_string(x); }
};

template <>
struct ToSql<unsigned long> {
  static_assert(sizeof(unsigned long) == 8,
                "We assume unsigned longs are 8 bytes.");
  // The maximum unsigned long is 18,446,744,073,709,551,615 which is 20 digits
  // of precision.
  std::string Type() { return "numeric(20)"; }
  std::string Value(unsigned long x) { return std::to_string(x); }
};

template <>
struct ToSql<unsigned long long> {
  static_assert(sizeof(unsigned long long) == 8,
                "We assume unsigned long longs are 8 bytes.");
  // The maximum unsigned long long is 18,446,744,073,709,551,615 which is 20
  // digits of precision.
  std::string Type() { return "numeric(20)"; }
  std::string Value(unsigned long long x) { return std::to_string(x); }
};

// TODO(mwhittaker): Ensure that we're not losing precision.
template <>
struct ToSql<float> {
  std::string Type() { return "real"; }
  std::string Value(float x) { return std::to_string(x); }
};

// TODO(mwhittaker): Ensure that we're not losing precision.
template <>
struct ToSql<double> {
  std::string Type() { return "double precision"; }
  std::string Value(double x) { return std::to_string(x); }
};

template <typename T>
struct ToSql<std::vector<T>> {
  std::string Type() { return fmt::format("{}[]", ToSql<T>().Type()); }

  std::string Value(const std::vector<T>& xs) {
    std::vector<std::string> values;
    for (const T& x : xs) {
      values.push_back(ToSql<T>().Value(x));
    }
    return fmt::format("ARRAY[]::{}[{}]", ToSql<T>().Type(),
                       common::Join(values));
  }
};

template <typename T, std::size_t N>
struct ToSql<std::array<T, N>> {
  std::string Type() { return fmt::format("{}[]", ToSql<T>().Type()); }

  std::string Value(const std::array<T, N>& xs) {
    std::vector<std::string> values;
    for (const T& x : xs) {
      values.push_back(ToSql<T>().Value(x));
    }
    return fmt::format("ARRAY[]::{}[{}]", ToSql<T>().Type(),
                       common::Join(values));
  }
};

template <typename Clock>
struct ToSql<std::chrono::time_point<Clock>> {
  std::string Type() { return "timestamp with time zone"; }

  // See https://www.lineagedbql.org/docs/current/static/functions-datetime.html
  // for more information.
  std::string Value(const std::chrono::time_point<Clock>& t) {
    return fmt::format(
        "TIMESTAMP WITH TIME ZONE 'epoch' + {} * INTERVAL '1 microsecond'",
        std::chrono::duration_cast<std::chrono::microseconds>(
            t.time_since_epoch())
            .count());
  }
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_TO_SQL_H_
