#ifndef POSTGRES_MOCK_TO_SQL_H_
#define POSTGRES_MOCK_TO_SQL_H_

#include <cstdint>

#include <string>
#include <type_traits>

#include "fmt/format.h"

namespace fluent {
namespace postgres {

// DO_NOT_SUBMIT(mwhittaker): Document.
template <typename T>
struct MockToSql;

template <>
struct MockToSql<bool> {
  std::string Type() { return "bool"; }
  std::string Value(bool b) { return b ? "true" : "false"; }
};

template <>
struct MockToSql<char> {
  std::string Type() { return "char"; }
  std::string Value(char c) { return std::string(1, c); }
};

template <>
struct MockToSql<std::string> {
  std::string Type() { return "string"; }
  std::string Value(const std::string& s) { return s; }
};

template <>
struct MockToSql<short int> {
  std::string Type() { return "short int"; }
  std::string Value(short int x) { return std::to_string(x); }
};

template <>
struct MockToSql<int> {
  std::string Type() { return "int"; }
  std::string Value(int x) { return std::to_string(x); }
};

template <>
struct MockToSql<long> {
  std::string Type() { return "long"; }
  std::string Value(long x) { return std::to_string(x); }
};

template <>
struct MockToSql<long long> {
  std::string Type() { return "long long"; }
  std::string Value(long long x) { return std::to_string(x); }
};

template <>
struct MockToSql<unsigned long> {
  std::string Type() { return "unsigned long"; }
  std::string Value(unsigned long x) { return std::to_string(x); }
};

template <>
struct MockToSql<unsigned long long> {
  std::string Type() { return "unsigned long long"; }
  std::string Value(unsigned long long x) { return std::to_string(x); }
};

template <>
struct MockToSql<float> {
  std::string Type() { return "float"; }
  std::string Value(float x) { return std::to_string(x); }
};

template <>
struct MockToSql<double> {
  std::string Type() { return "double"; }
  std::string Value(double x) { return std::to_string(x); }
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_MOCK_TO_SQL_H_
