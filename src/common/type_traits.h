#ifndef COMMON_TYPE_TRAITS_H_
#define COMMON_TYPE_TRAITS_H_

#include <cstddef>

#include <set>
#include <string>
#include <type_traits>
#include <vector>

// C++ type traits (like the ones found in <type_traits>) are template
// metaprogramming utilities that can be used to operate over types. This file
// implements a couple of useful type traits that are missing from the standard
// <type_traits> header.

namespace fluent {

// sizet_constant
template <std::size_t I>
using sizet_constant = std::integral_constant<std::size_t, I>;

// Boolean operations
template <typename A, typename B>
struct And : public std::integral_constant<bool, A::value && B::value> {};

template <typename A, typename B>
struct Or : public std::integral_constant<bool, A::value || B::value> {};

template <typename A>
struct Not : public std::integral_constant<bool, !A::value> {};

// Arithmetic operations
template <typename Lhs, typename Rhs>
struct Lt : public std::integral_constant<bool, (Lhs::value < Rhs::value)> {};

template <typename Lhs, typename Rhs>
struct Le : public std::integral_constant<bool, Lhs::value <= Rhs::value> {};

template <typename Lhs, typename Rhs>
struct Eq : public std::integral_constant<bool, Lhs::value == Rhs::value> {};

template <typename Lhs, typename Rhs>
struct Ne : public std::integral_constant<bool, Lhs::value != Rhs::value> {};

template <typename Lhs, typename Rhs>
struct Gt : public std::integral_constant<bool, (Lhs::value > Rhs::value)> {};

template <typename Lhs, typename Rhs>
struct Ge : public std::integral_constant<bool, Lhs::value >= Rhs::value> {};

// All<>::value == true
// All<std::true_type>::value == true
// All<std::false_type>::value == false
// All<std::true_type, std::true_type>::value == true
// All<std::true_type, std::false_type>::value == false
template <typename... Ts>
struct All;

template <>
struct All<> : public std::true_type {};

template <typename T, typename... Ts>
struct All<T, Ts...>
    : public std::integral_constant<bool, T::value && All<Ts...>::value> {};

// Any<>::value == false
// Any<std::true_type>::value == true
// Any<std::false_type>::value == false
// Any<std::true_type, std::true_type>::value == true
// Any<std::true_type, std::false_type>::value == true
template <typename... Ts>
struct Any;

template <>
struct Any<> : public std::false_type {};

template <typename T, typename... Ts>
struct Any<T, Ts...>
    : public std::integral_constant<bool, T::value || Any<Ts...>::value> {};

// InRange<0, 0, 5>::value == true
// InRange<1, 0, 5>::value == true
// InRange<2, 0, 5>::value == true
// InRange<3, 0, 5>::value == true
// InRange<4, 0, 5>::value == true
// InRange<5, 0, 5>::value == false
template <std::size_t I, std::size_t LowInclusive, std::size_t HighExclusive>
struct InRange : public std::integral_constant<bool, (LowInclusive <= I &&
                                                      I < HighExclusive)> {};

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

// Unwrap<Template<T, ...>>::type == T
template <typename T>
struct Unwrap;

template <template <typename...> class Template, typename T, typename... Ts>
struct Unwrap<Template<T, Ts...>> {
  using type = T;
};

// http://en.cppreference.com/w/cpp/types/is_invocable.
// https://stackoverflow.com/a/24233730/3187068
template <typename F, typename... Args>
struct IsInvocable {
 private:
  template <typename FF, typename... AA>
  static constexpr auto check(int)
      -> decltype(std::declval<FF>()(std::declval<AA>()...), std::true_type());

  template <typename FF, typename... AA>
  static constexpr std::false_type check(...);

 public:
  static constexpr bool value = decltype(check<F, Args...>(0))();
};

}  // namespace fluent

#endif  //  COMMON_TYPE_TRAITS_H_
