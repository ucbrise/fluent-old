#ifndef COMMON_TYPE_LIST_H_
#define COMMON_TYPE_LIST_H_

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <utility>

#include "common/type_traits.h"

namespace fluent {

// TODO(mwhittaker): Namespace these functions so they aren't so long?

// A quick review of template metaprogramming. In normal programming, programs
// manipulate values like 42, true, "foo" etc. Functions take in zero or more
// values and output a single value. For example, consider the function foo:
//
//   int foo(int x) {
//     return x + x;
//   }
//
// which we can call like this:
//
//   int x = f(42);
//
// In template metaprogramming, metaprograms manipulate types like int, bool
// std::string. Class templates play the role of functions. They take in zero
// or more types as type parameters and output a single type as a type alias.
// This is best seen through an example. Consider the following class template:
//
//   template <typename T>
//   struct F {
//     using type = T*;
//   };
//
// which can be "invoked" like this:
//
//   using x = F<int>::type;
//
// A TypeList is the metaprogram equivalent of a list. It contains types. This
// file contains a TypeList struct and functions to operate over it. For
// example, the following program:
//
//   vector<int> xs = {1, 2, 3};
//   vector<int> ys = {1, 2, 3};
//   assert(len(xs) == 3);
//   assert(project(xs, 2) == {3});
//   assert(take(xs, 2) == {1, 2});
//   assert(concat(xs, ys) == {1, 2, 3, 1, 2, 3});
//
// would correspond to the following metaprogram:
//
//   using xs = TypeList<int, char, float>;
//   using ys = TypeList<int, char, float>;
//   static_assert(TypeListLen<xs>::value == 3);
//   static_assert(TypeListProject<xs, 2>::type == TypeList<float>);
//   static_assert(TypeListTake<xs, 2>::type == TypeList<int, char>);
//   static_assert(TypeListConcat<xs, ys>::type ==
//                 TypeList<int, char, float, int, char, float>);

// http://en.cppreference.com/w/cpp/utility/tuple/tuple_element
template <typename... Ts>
struct TypeList {
  template <std::size_t N>
  using type = typename std::tuple_element<N, std::tuple<Ts...>>::type;
};

// Map
template <typename TypeList, template <typename> class F>
struct TypeListMap;

template <typename... Ts, template <typename> class F>
struct TypeListMap<TypeList<Ts...>, F> {
  using type = TypeList<typename F<Ts>::type...>;
};

// Concat
template <typename LeftTypeList, typename RightTypeList>
struct TypeListConcat;

template <typename... Ls, typename... Rs>
struct TypeListConcat<TypeList<Ls...>, TypeList<Rs...>> {
  using type = TypeList<Ls..., Rs...>;
};

// Project
template <typename TypeList, std::size_t... Is>
struct TypeListProject;

template <typename... Ts, std::size_t... Is>
struct TypeListProject<TypeList<Ts...>, Is...> {
  using type = TypeList<typename TypeList<Ts...>::template type<Is>...>;
};

// Take
template <typename TypeList, typename IndexSequence>
struct TypeListTakeImpl;

template <typename... Ts, std::size_t... Is>
struct TypeListTakeImpl<TypeList<Ts...>, std::index_sequence<Is...>> {
  using type = typename TypeListProject<TypeList<Ts...>, Is...>::type;
};

template <typename TypeList, std::size_t N>
struct TypeListTake;

template <typename... Ts, std::size_t N>
struct TypeListTake<TypeList<Ts...>, N> {
  using index_sequence = std::make_index_sequence<std::min(N, sizeof...(Ts))>;
  using type = typename TypeListTakeImpl<TypeList<Ts...>, index_sequence>::type;
};

// Drop
template <typename TypeList, std::size_t N>
struct TypeListDrop;

template <std::size_t N>
struct TypeListDrop<TypeList<>, N> {
  using type = TypeList<>;
};

template <typename T, typename... Ts>
struct TypeListDrop<TypeList<T, Ts...>, 0> {
  using type = TypeList<T, Ts...>;
};

template <typename T, typename... Ts, std::size_t N>
struct TypeListDrop<TypeList<T, Ts...>, N> {
  using type = typename TypeListDrop<TypeList<Ts...>, N - 1>::type;
};

// Len
template <typename TypeList>
struct TypeListLen;

template <typename... Ts>
struct TypeListLen<TypeList<Ts...>>
    : public std::integral_constant<std::size_t, sizeof...(Ts)> {};

// AllSame
template <typename TypeList>
struct TypeListAllSame;

template <>
struct TypeListAllSame<TypeList<>> : public std::true_type {};

template <typename T>
struct TypeListAllSame<TypeList<T>> : public std::true_type {};

template <typename T, typename U, typename... Us>
struct TypeListAllSame<TypeList<T, U, Us...>> {
  static constexpr bool value =
      std::is_same<T, U>::value && TypeListAllSame<TypeList<U, Us...>>::value;
};

// TypeListTo<Template, TypeList<Ts...>> = Template<Ts...>
template <template <typename...> class Template, typename TypeList>
struct TypeListTo;

template <template <typename...> class Template, typename... Ts>
struct TypeListTo<Template, TypeList<Ts...>> {
  using type = Template<Ts...>;
};

// TypeListFrom<Template<Ts...>> = TypeList<Ts...>
template <typename From>
struct TypeListFrom;

template <template <typename...> class Template, typename... Ts>
struct TypeListFrom<Template<Ts...>> {
  using type = TypeList<Ts...>;
};

// TypeList -> Tuple
template <typename T>
struct TypeListToTuple {
  static_assert(IsTemplate<TypeList, T>::value, "");
  using type = typename TypeListTo<std::tuple, T>::type;
};

// Tuple -> TypeList
template <typename Tuple>
struct TupleToTypeList {
  static_assert(IsTemplate<std::tuple, Tuple>::value, "");
  using type = typename TypeListFrom<Tuple>::type;
};

}  // namespace fluent

#endif  //  COMMON_TYPE_LIST_H_
