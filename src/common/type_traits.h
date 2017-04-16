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

// `IsTemplate<Template, T>::value` returns whether `T` is an instantiation of
// the template `Template`. For example,
//
//   IsTemplate<std::set, std::set<int>>::value;       // true
//   IsTemplate<std::vector, std::vector<int>>::value; // true
//   IsTemplate<std::set, std::vector<int>>::value;    // false
//   IsTemplate<std::set, int>::value;                 // false
//
// Note that if `T` has any modifiers (e.g. const, volatile, reference,
// pointer), `IsTemplate` will return false. You can use `std::decay` to strip
// modifiers if you want. For example,
//
//   IsTemplate<std::set, const std::set<int>>::value // false
//   IsTemplate<std::set, std::set<int>&>::value      // false
template <template <typename...> class Template, typename T>
struct IsTemplate : public std::false_type {};

template <template <typename...> class Template, typename... Ts>
struct IsTemplate<Template, Template<Ts...>> : public std::true_type {};

// IsSet<T> == IsTemplate<std::set, T>.
template <typename T>
using IsSet = IsTemplate<std::set, T>;

// IsVector<T> == IsTemplate<std::vector, T>.
template <typename T>
using IsVector = IsTemplate<std::vector, T>;

// IsTuple<T> == IsTemplate<std::tuple, T>.
template <typename T>
using IsTuple = IsTemplate<std::tuple, T>;

// IsPair<T> == IsTemplate<std::pair, T>.
template <typename T>
using IsPair = IsTemplate<std::pair, T>;

}  // namespace fluent

#endif  //  COMMON_TYPE_TRAITS_H_