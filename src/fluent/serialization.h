#ifndef FLUENT_SERIALIZATION_H_
#define FLUENT_SERIALIZATION_H_

#include <sstream>
#include <string>

#include "glog/logging.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/utility.hpp>

#include "common/tuple_util.h"
#include "common/type_traits.h"

namespace fluent {

// The `Collection<T1, ..., Tn>` class provides a `GetParser` method which
// parses a vector of `n` strings into a tuple of type `tuple<T1, ..., Tn>` and
// inserts it into the collection. To parse strings into types, the
// `Collection` template assumes that a `T FromString<T>(const std::string&)`
// function exists for `T1`, ..., `Tn`.
//
// Similarly, a `Channel<T1, ..., Tn>` assumes the existence of a `std::string
// ToString<T>(const T& x)` function in order to serialize tuples and send them
// over the network.
//
// This file contains `ToString` and `FromString` implementations for most of
// the basic C++ types (e.g. int, float, string, char, etc.). If you ever want
// to use a custom type in a Collection, you'll have to define your own
// `ToString` and `FromString` functions elsewhere and have them found with
// argument-dependent lookup.
//
// TODO(mwhittaker): Give an example of defining your own `ToString` and
// `FromString` functions.

template <typename T>
std::string ToString(const T& x) {
  std::stringstream ss;
  boost::archive::text_oarchive oarch(ss);
  oarch << x;
  return ss.str();
}

template <typename T>
T FromString(const std::string& s) {
  std::stringstream ss;
  ss << s;
  boost::archive::text_iarchive iarch(ss);
  T res;
  iarch >> res;
  return res;
}

// std::string
template <>
std::string ToString(const std::string& s) {
  return s;
}

template <>
std::string FromString<std::string>(const std::string& s) {
  return s;
}

// char
template <>
std::string ToString(const char& c) {
  return std::string(1, c);
}

template <>
char FromString<char>(const std::string& s) {
  return s[0];
}

// int
template <>
std::string ToString(const int& i) {
  return std::to_string(i);
}

template <>
int FromString<int>(const std::string& s) {
  return std::stoi(s);
}

// long
template <>
std::string ToString(const long& l) {
  return std::to_string(l);
}

template <>
long FromString<long>(const std::string& s) {
  return std::stol(s);
}

// long long
template <>
std::string ToString(const long long& ll) {
  return std::to_string(ll);
}

template <>
long long FromString<long long>(const std::string& s) {
  return std::stoll(s);
}

// unsigned long
template <>
std::string ToString(const unsigned long& ul) {
  return std::to_string(ul);
}

template <>
unsigned long FromString<unsigned long>(const std::string& s) {
  return std::stoul(s);
}

// unsigned long long
template <>
std::string ToString(const unsigned long long& ull) {
  return std::to_string(ull);
}

template <>
unsigned long long FromString<unsigned long long>(const std::string& s) {
  return std::stoull(s);
}

// float
template <>
std::string ToString(const float& f) {
  return std::to_string(f);
}

template <>
float FromString<float>(const std::string& s) {
  return std::stof(s);
}

// double
template <>
std::string ToString(const double& d) {
  return std::to_string(d);
}

template <>
double FromString<double>(const std::string& s) {
  return std::stod(s);
}

// long double
template <>
std::string ToString(const long double& ld) {
  return std::to_string(ld);
}

template <>
long double FromString<long double>(const std::string& s) {
  return std::stold(s);
}

}  // namespace fluent

namespace boost {
namespace serialization {

template<typename Archive, typename... Ts>
void serialize(Archive& ar, std::tuple<Ts...>& t, const unsigned int) {
    fluent::TupleIter(t, [&ar](auto& x) { ar & x; });
}

} // namespace serialization
} // namespace boost

#endif  // FLUENT_SERIALIZATION_H_
