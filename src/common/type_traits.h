#ifndef COMMON_TYPE_TRAITS_H_
#define COMMON_TYPE_TRAITS_H_

#include <set>
#include <type_traits>
#include <vector>

// C++ type traits (like the ones found in <type_traits>) are template
// metaprogramming utilities that can be used to operate over types. This file
// implements a couple of useful type traits that are missing from the standard
// <type_traits> header.

namespace fluent {

// `IsSet<T>::value` returns whether `T` is `std::set<U>` for some `U`. Note
// that if `T` has any modifiers (e.g. const, volatile, reference, pointer),
// `IsSet` will return false. You can use `std::decay` to strip modifiers if
// you want.
//
//   IsSet<std::set<int>>::value;      // true
//   IsSet<std::set<float>>::value;    // true
//   IsSet<int>::value                 // false
//   IsSet<const std::set<int>>::value // false
//   IsSet<std::set<int>&>::value      // false
template <typename T>
struct IsSet : public std::false_type {};

template <typename... Ts>
struct IsSet<std::set<Ts...>> : public std::true_type {};

// `IsVector<T>::value` returns whether `T` is `std::vector<U>` for some `U`.
// As with `IsSet`, if `T` has any modifiers `IsVector` will return false.
//
//   IsVector<std::vector<int>>::value;      // true
//   IsVector<std::vector<float>>::value;    // true
//   IsVector<int>::value                    // false
//   IsVector<const std::vector<int>>::value // false
//   IsVector<std::vector<int>&>::value      // false
template <typename T>
struct IsVector : public std::false_type {};

template <typename... Ts>
struct IsVector<std::vector<Ts...>> : public std::true_type {};

}  // namespace fluent

#endif  //  COMMON_TYPE_TRAITS_H_
