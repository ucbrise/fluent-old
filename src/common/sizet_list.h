#ifndef COMMON_SIZET_LIST_H_
#define COMMON_SIZET_LIST_H_

#include <cstddef>

#include <utility>

#include "common/type_list.h"
#include "common/type_traits.h"

namespace fluent {

// A SizetList<Is...> is analagous to a TypeList<Ts...>, except that a
// SizetList holds a bunch of std::size_t and a TypeList holds a bunch of
// std::size_t.
template <std::size_t... Is>
struct SizetList {};

// SizetListToTypeList<SizetList<1, ..., n>> =
//   TypeList<sizet_constant<1>, ..., sizet_constant<n>>
template <typename SizetList>
struct SizetListToTypeList;

template <std::size_t... Is>
struct SizetListToTypeList<SizetList<Is...>> {
  using type = TypeList<std::integral_constant<std::size_t, Is>...>;
};

// TypeListToSizetList<TypeList<sizet_constant<1>, ..., sizet_constant<n>>> =
//   SizetList<1, ..., n>
template <typename TypeList>
struct TypeListToSizetList;

template <std::size_t... Is>
struct TypeListToSizetList<
    TypeList<std::integral_constant<std::size_t, Is>...>> {
  using type = SizetList<Is...>;
};

// SizetListTo<Template, SizetList<Is...>> = Template<Is...>
template <template <std::size_t...> class Template, typename SizetList>
struct SizetListTo;

template <template <std::size_t...> class Template, std::size_t... Is>
struct SizetListTo<Template, SizetList<Is...>> {
  using type = Template<Is...>;
};

// SizetListFrom<Template<Is...>> = SizetList<Is...>
template <typename From>
struct SizetListFrom;

template <template <std::size_t...> class Template, std::size_t... Is>
struct SizetListFrom<Template<Is...>> {
  using type = SizetList<Is...>;
};

// SizetListFromIndexSequence<index_sequence<size_t, Is...>> = SizetList<Is...>
template <typename IndexSequence>
struct SizetListFromIndexSequence;

template <std::size_t... Is>
struct SizetListFromIndexSequence<std::index_sequence<Is...>> {
  using type = SizetList<Is...>;
};

// Take
template <typename SizetList, std::size_t N>
struct SizetListTake;

template <std::size_t... Is, std::size_t N>
struct SizetListTake<SizetList<Is...>, N> {
  using type_list = typename SizetListToTypeList<SizetList<Is...>>::type;
  using taken = typename TypeListTake<type_list, N>::type;
  using type = typename TypeListToSizetList<taken>::type;
};

// Drop
template <typename SizetList, std::size_t N>
struct SizetListDrop;

template <std::size_t... Is, std::size_t N>
struct SizetListDrop<SizetList<Is...>, N> {
  using type_list = typename SizetListToTypeList<SizetList<Is...>>::type;
  using dropped = typename TypeListDrop<type_list, N>::type;
  using type = typename TypeListToSizetList<dropped>::type;
};

// Range
template <std::size_t LowInclusive, std::size_t HighExclusive>
struct SizetListRange {
  using index_sequence = std::make_index_sequence<HighExclusive>;
  using sizet_list = typename SizetListFromIndexSequence<index_sequence>::type;
  using type = typename SizetListDrop<sizet_list, LowInclusive>::type;
};

}  // namespace fluent

#endif  //  COMMON_SIZET_LIST_H_
