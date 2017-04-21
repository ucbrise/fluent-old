#ifndef COMMON_SIZET_LIST_H_
#define COMMON_SIZET_LIST_H_

#include <cstddef>

namespace fluent {

// A SizetList<Is...> is analagous to a TypeList<Ts...>, except that a
// SizetList holds a bunch of std::size_t and a TypeList holds a bunch of
// std::size_t.
template <std::size_t... Is>
struct SizetList {};

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

}  // namespace fluent

#endif  //  COMMON_SIZET_LIST_H_
