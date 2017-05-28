#ifndef RA_LOGICAL_GROUP_BY_H_
#define RA_LOGICAL_GROUP_BY_H_

#include <cstddef>

#include <type_traits>

#include "common/sizet_list.h"
#include "common/static_assert.h"
#include "common/tuple_util.h"
#include "common/type_list.h"
#include "common/type_traits.h"
#include "ra/aggregates.h"
#include "ra/keys.h"
#include "ra/logical/logical_ra.h"

namespace fluent {
namespace ra {
namespace logical {
namespace detail {

// ProjectBySizetList
template <typename Typelist, typename SizetList>
struct TypeListProjectBySizetList;

template <typename... Ts, std::size_t... Is>
struct TypeListProjectBySizetList<TypeList<Ts...>, SizetList<Is...>> {
  using type = typename TypeListProject<TypeList<Ts...>, Is...>::type;
};

// TypeOfGet
template <typename T>
struct TypeOfGet {
  using type = typename std::decay<decltype(std::declval<T>().Get())>::type;
};

}  // namespace detail

template <typename Ra, typename Keys, typename... Aggregates>
struct GroupBy;

template <typename Ra, std::size_t... Ks, typename... Aggregates>
struct GroupBy<Ra, Keys<Ks...>, Aggregates...> : public LogicalRa {
  static_assert(StaticAssert<std::is_base_of<LogicalRa, Ra>>::value, "");
  using child_column_types = typename Ra::column_types;
  using child_len_t = typename TypeListLen<child_column_types>::type;
  static constexpr std::size_t child_len = child_len_t::value;
  static_assert(StaticAssert<All<InRange<Ks, 0, child_len>...>>::value, "");
  static_assert(
      StaticAssert<All<std::is_base_of<agg::Aggregate, Aggregates>...>>::value,
      "");
  using key_types = typename TypeListProject<child_column_types, Ks...>::type;
  using aggregate_impl_types = TypeList<  //
      typename Aggregates::template type<
          typename detail::TypeListProjectBySizetList<
              child_column_types,
              typename SizetListFrom<Aggregates>::type>::type>...  //
      >;
  using aggregate_types =
      typename TypeListMap<aggregate_impl_types, detail::TypeOfGet>::type;

  using column_types =
      typename TypeListConcat<key_types, aggregate_types>::type;
  explicit GroupBy(Ra child_) : child(std::move(child_)) {}
  Ra child;
};

template <typename Keys, typename... Aggregates>
struct group_by;

template <std::size_t... Ks, typename... Aggregates>
struct group_by<Keys<Ks...>, Aggregates...> {};

template <typename Ra, std::size_t... Ks, typename... Aggregates,
          typename RaDecayed = typename std::decay<Ra>::type>
GroupBy<RaDecayed, Keys<Ks...>, Aggregates...> operator|(
    Ra&& child, group_by<Keys<Ks...>, Aggregates...>) {
  return GroupBy<RaDecayed, Keys<Ks...>, Aggregates...>(
      std::forward<Ra&&>(child));
}

}  // namespace logical
}  // namespace ra
}  // namespace fluent

#endif  // RA_LOGICAL_GROUP_BY_H_
