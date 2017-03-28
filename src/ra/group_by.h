#ifndef RA_GROUP_BY_H_
#define RA_GROUP_BY_H_

#include <cstddef>

#include <map>
#include <type_traits>
#include <utility>

#include "fmt/format.h"
#include "range/v3/all.hpp"

#include "common/string_util.h"
#include "common/type_list.h"

// TODO(mwhittaker): Document this file.

namespace fluent {
namespace ra {

template <std::size_t... Is>
struct Keys {};

namespace detail {

template <typename T>
struct TypeOfGet {
  using type = typename std::decay<decltype(std::declval<T>().Get())>::type;
};

template <std::size_t... Ks, typename... Ts>
auto Project(Keys<Ks...>, const std::tuple<Ts...>& t) {
  return std::tuple_cat(std::make_tuple(std::get<Ks>(t))...);
}

}  // namespace detail

template <typename PhysicalChild, typename LogicalGroupBy>
class PhysicalGroupBy {
 public:
  using keys = typename LogicalGroupBy::keys;
  using key_types = typename LogicalGroupBy::key_types;
  using key_tuple = typename TypeListToTuple<key_types>::type;
  using child_column_types = typename LogicalGroupBy::child_column_types;
  using child_column_tuple = typename TypeListToTuple<child_column_types>::type;

  using aggregate_impl_types = typename LogicalGroupBy::aggregate_impl_types;
  using aggregate_impl_tuple =
      typename TypeListToTuple<aggregate_impl_types>::type;

  using aggregate_types = typename LogicalGroupBy::aggregate_types;
  using aggregate_tuple = typename TypeListToTuple<aggregate_types>::type;

  explicit PhysicalGroupBy(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    if (groups_.size() == 0) {
      auto rng = child_.ToRange();
      for (auto iter = ranges::begin(rng); iter != ranges::end(rng); ++iter) {
        auto key = detail::Project(keys(), *iter);
        Update<0>(&groups_[key], *iter);
      }
    }

    return ranges::view::all(groups_) |
           ranges::view::transform([this](const auto& pair) {
             return std::tuple_cat(
                 pair.first,
                 ToAggregate(pair.second,
                             std::make_index_sequence<
                                 TypeListLen<aggregate_impl_types>::value>()));
           });
  }

 private:
  template <template <std::size_t, typename> class AggregateImpl,
            std::size_t AggregateColumn, typename T>
  void UpdateAgg(AggregateImpl<AggregateColumn, T>* agg,
                 const child_column_tuple& t) {
    agg->Update(std::get<AggregateColumn>(t));
  }

  template <std::size_t I>
  typename std::enable_if<I == TypeListLen<aggregate_types>::value>::type
  Update(aggregate_impl_tuple*, const child_column_tuple&) {}

  template <std::size_t I>
  typename std::enable_if<I != TypeListLen<aggregate_types>::value>::type
  Update(aggregate_impl_tuple* agg, const child_column_tuple& t) {
    UpdateAgg(&std::get<I>(*agg), t);
    Update<I + 1>(agg, t);
  }

  template <std::size_t... Is>
  aggregate_tuple ToAggregate(const aggregate_impl_tuple& aggs,
                              std::index_sequence<Is...>) {
    return std::make_tuple(std::get<Is>(aggs).Get()...);
  }

  PhysicalChild child_;
  std::map<key_tuple, aggregate_impl_tuple> groups_;
};

template <typename LogicalChild, typename KeyColumns, typename... Aggregates>
class GroupBy;

template <typename LogicalChild, std::size_t... Ks,
          template <std::size_t> class... Aggregates,
          std::size_t... AggregateColumns>
class GroupBy<LogicalChild, Keys<Ks...>, Aggregates<AggregateColumns>...> {
 public:
  using keys = Keys<Ks...>;
  using child_column_types = typename LogicalChild::column_types;
  using key_types = typename TypeListProject<child_column_types, Ks...>::type;
  using aggregate_impl_types =
      TypeList<typename Aggregates<AggregateColumns>::template type<
          typename child_column_types::template type<AggregateColumns>>...>;
  using aggregate_types =
      typename TypeListMap<aggregate_impl_types, detail::TypeOfGet>::type;
  using column_types =
      typename TypeListConcat<key_types, aggregate_types>::type;

  explicit GroupBy(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const {
    return PhysicalGroupBy<
        decltype(child_.ToPhysical()),
        GroupBy<LogicalChild, Keys<Ks...>, Aggregates<AggregateColumns>...>>(
        child_.ToPhysical());
  }

  std::string ToDebugString() const {
    return fmt::format("GroupBy<Keys<{}>, {}>({})", Join(Ks...),
                       Join(Aggregates<AggregateColumns>::ToDebugString()...),
                       child_.ToDebugString());
  }

 private:
  LogicalChild child_;
};

template <typename KeyColumns, typename... Aggregates>
struct group_by;

template <std::size_t... Ks, template <std::size_t> class... Aggregates,
          std::size_t... AggregateColumns>
struct group_by<Keys<Ks...>, Aggregates<AggregateColumns>...> {};

template <typename LogicalChild, std::size_t... Ks,
          template <std::size_t> class... Aggregates,
          std::size_t... AggregateColumns>
GroupBy<typename std::decay<LogicalChild>::type, Keys<Ks...>,
        Aggregates<AggregateColumns>...>
operator|(LogicalChild&& child,
          group_by<Keys<Ks...>, Aggregates<AggregateColumns>...>) {
  return GroupBy<typename std::decay<LogicalChild>::type, Keys<Ks...>,
                 Aggregates<AggregateColumns>...>(
      std::forward<LogicalChild&&>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_GROUP_BY_H_
