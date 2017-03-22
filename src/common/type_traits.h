#ifndef COMMON_TYPE_TRAITS_H_
#define COMMON_TYPE_TRAITS_H_

#include <set>
#include <type_traits>
#include <vector>

namespace fluent {

template <typename T>
struct IsSet : public std::false_type {};

template <typename... Ts>
struct IsSet<std::set<Ts...>> : public std::true_type {};

template <typename T>
struct IsVector : public std::false_type {};

template <typename... Ts>
struct IsVector<std::vector<Ts...>> : public std::true_type {};

}  // namespace fluent

#endif  //  COMMON_TYPE_TRAITS_H_
