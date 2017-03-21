#ifndef POSTGRES_SQL_TYPE_H_
#define POSTGRES_SQL_TYPE_H_

#include <cstdint>

#include <chrono>
#include <string>
#include <type_traits>

#include "fmt/format.h"

// DO_NOT_SUBMIT(mwhittaker): Document.
namespace fluent {
namespace postgres {

template <typename T>
struct SqlType;

template <>
struct SqlType<bool> {
  std::string type() { return "boolean"; }
  std::string value(bool b) { return b ? "true" : "false"; }
};

template <>
struct SqlType<char> {
  std::string type() { return "char(1)"; }
  std::string value(char c) { return fmt::format("'{}'", c); }
};

template <>
struct SqlType<std::string> {
  std::string type() { return "text"; }
  std::string value(const std::string& s) { return fmt::format("'{}'", s); }
};

template <>
struct SqlType<short int> {
  static_assert(sizeof(short int) == 2, "We assume short ints are 2 bytes.");
  std::string type() { return "smallint"; }
  std::string value(short int x) { return std::to_string(x); }
};

template <>
struct SqlType<int> {
  static_assert(sizeof(int) == 4, "We assume ints are 4 bytes.");
  std::string type() { return "integer"; }
  std::string value(int x) { return std::to_string(x); }
};

template <>
struct SqlType<long> {
  static_assert(sizeof(long) == 8, "We assume longs are 8 bytes.");
  std::string type() { return "bigint"; }
  std::string value(long x) { return std::to_string(x); }
};

template <>
struct SqlType<long long> {
  static_assert(sizeof(long long) == 8, "We assume long longs are 8 bytes.");
  std::string type() { return "bigint"; }
  std::string value(long long x) { return std::to_string(x); }
};

template <>
struct SqlType<unsigned long> {
  static_assert(sizeof(unsigned long) == 8,
                "We assume unsigned longs are 8 bytes.");
  // The maximum unsigned long is 18,446,744,073,709,551,615 which is 20 digits
  // of precision.
  std::string type() { return "numeric(20)"; }
  std::string value(unsigned long x) { return std::to_string(x); }
};

template <>
struct SqlType<unsigned long long> {
  static_assert(sizeof(unsigned long long) == 8,
                "We assume unsigned long longs are 8 bytes.");
  // The maximum unsigned long long is 18,446,744,073,709,551,615 which is 20
  // digits
  // of precision.
  std::string type() { return "numeric(20)"; }
  std::string value(unsigned long long x) { return std::to_string(x); }
};

// TODO(mwhittaker): Ensure that we're not losing precision.
template <>
struct SqlType<float> {
  std::string type() { return "real"; }
  std::string value(float x) { return std::to_string(x); }
};

// TODO(mwhittaker): Ensure that we're not losing precision.
template <>
struct SqlType<double> {
  std::string type() { return "double precision"; }
  std::string value(double x) { return std::to_string(x); }
};

template <typename Clock>
struct SqlType<std::chrono::time_point<Clock>> {
  std::string type() { return "timestamp with time zone"; }
  std::string value(const std::chrono::time_point<Clock>& t) {
    return fmt::format(
        "TIMESTAMP WITH TIME ZONE 'epoch' + {} * INTERVAL '1 microsecond'",
        std::chrono::duration_cast<std::chrono::microseconds>(
            t.time_since_epoch())
            .count());
  }
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_SQL_TYPE_H_