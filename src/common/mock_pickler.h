#ifndef COMMON_MOCK_PICKLER_H_
#define COMMON_MOCK_PICKLER_H_

#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

#include "glog/logging.h"

namespace fluent {
namespace common {

// TODO(mwhittaker): Change implementations of float, double, and long double
// so that we don't lose precision.
// TODO(mwhittaker): Change Load to return a StatusOr.
// TODO(mwhittaker): Implement a real pickler using something like boost or
// protobuf.

// A pickler (synonymously serializer or marshaller) is a struct which convert
// C++ objects to and from strings. The MockPickler is a dirt simple pickler
// which pickles things as simply as possible with no regard for performance or
// space efficiency. For example:
//
//   MockPickler<int> int_pickler;
//   int_pickler.Dump(42);   // "1"
//   int_pickler.Load("42"); // 42
template <typename T>
struct MockPickler;

// string
template <>
struct MockPickler<std::string> {
  std::string Dump(const std::string& s) { return s; }
  std::string Load(const std::string& s) { return s; }
};

// char
template <>
struct MockPickler<char> {
  std::string Dump(char c) { return std::string(1, c); }
  char Load(const std::string& s) { return s[0]; }
};

// bool
template <>
struct MockPickler<bool> {
  std::string Dump(bool b) { return b ? "true" : "false"; }

  bool Load(const std::string& s) {
    CHECK(s == "true" || s == "false") << s;
    return s == "true";
  }
};

// int
template <>
struct MockPickler<int> {
  std::string Dump(int x) { return std::to_string(x); }
  int Load(const std::string& s) { return std::stoi(s); }
};

// long
template <>
struct MockPickler<long> {
  std::string Dump(long x) { return std::to_string(x); }
  long Load(const std::string& s) { return std::stol(s); }
};

// long long
template <>
struct MockPickler<long long> {
  std::string Dump(long long x) { return std::to_string(x); }
  long long Load(const std::string& s) { return std::stoll(s); }
};

// unsigned long
template <>
struct MockPickler<unsigned long> {
  std::string Dump(unsigned long x) { return std::to_string(x); }
  unsigned long Load(const std::string& s) { return std::stoul(s); }
};

// unsigned long long
template <>
struct MockPickler<unsigned long long> {
  std::string Dump(unsigned long long x) { return std::to_string(x); }
  unsigned long long Load(const std::string& s) { return std::stoull(s); }
};

// float
template <>
struct MockPickler<float> {
  std::string Dump(float x) { return std::to_string(x); }
  float Load(const std::string& s) { return std::stof(s); }
};

// double
template <>
struct MockPickler<double> {
  std::string Dump(double x) { return std::to_string(x); }
  double Load(const std::string& s) { return std::stod(s); }
};

// long double
template <>
struct MockPickler<long double> {
  std::string Dump(long double x) { return std::to_string(x); }
  long double Load(const std::string& s) { return std::stold(s); }
};

}  // namespace common
}  // namespace fluent

#endif  // COMMON_MOCK_PICKLER_H_
