#ifndef FLUENT_SERIALIZATION_H_
#define FLUENT_SERIALIZATION_H_

#include <string>

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

// `ToString` dispatches to `std::to_string` by default.
template <typename T>
std::string ToString(const T& x) {
  return std::to_string(x);
}

// Unlike `ToString`, C++ doesn't provide a nice default implementation.
template <typename T>
T FromString(const std::string&);

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

//lww pair
template <typename K, typename V>
std::string ToString(const std::pair<K, V>& p) {
  std::string delimiter = "|";
  return ToString(std::get<0>(p)) + delimiter + ToString(std::get<1>(p));
}

template <>
char FromString<char>(const std::string& s) {
  return s[0];
}

// int
template <>
int FromString<int>(const std::string& s) {
  return std::stoi(s);
}

// long
template <>
long FromString<long>(const std::string& s) {
  return std::stol(s);
}

// long long
template <>
long long FromString<long long>(const std::string& s) {
  return std::stoll(s);
}

// unsigned long
template <>
unsigned long FromString<unsigned long>(const std::string& s) {
  return std::stoul(s);
}

// unsigned long long
template <>
unsigned long long FromString<unsigned long long>(const std::string& s) {
  return std::stoull(s);
}

// float
template <>
float FromString<float>(const std::string& s) {
  return std::stof(s);
}

// double
template <>
double FromString<double>(const std::string& s) {
  return std::stod(s);
}

// long double
template <>
long double FromString<long double>(const std::string& s) {
  return std::stold(s);
}

// lww pair for indy monitoring
template <>
std::pair<long, std::size_t> FromString<std::pair<long, std::size_t>>(const std::string& s) {
  std::string str = s;
  std::string delimiter = "|";
  std::string p1 = str.substr(0, str.find(delimiter));
  str.erase(0, str.find(delimiter) + delimiter.length());
  std::string p2 = str;
  return std::make_pair(FromString<long>(p1), FromString<std::size_t>(p2));
}

// template <typename T>
// typename std::enable_if<IsPair<T>::value, T>::type
// FromString(const std::string& s) {
//   std::string str = s;
//   std::string delimiter = "|";
//   std::string p1 = str.substr(0, str.find(delimiter));
//   str.erase(0, str.find(delimiter) + delimiter.length());
//   std::string p2 = str;
//   return T(FromString<T::first_type>(p1), FromString<T::second_type>(p2));
// }

}  // namespace fluent

#endif  // FLUENT_SERIALIZATION_H_
