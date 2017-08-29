#ifndef RA_LOGICAL_TO_PHYSICAL_H_
#define RA_LOGICAL_TO_PHYSICAL_H_

#include <cstddef>

#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "collections/collection_tuple_ids.h"
#include "common/static_assert.h"
#include "common/tuple_util.h"
#include "fluent/local_tuple_id.h"
#include "ra/aggregates.h"
#include "ra/logical/all.h"
#include "ra/logical/logical_ra.h"
#include "ra/physical/all.h"
#include "ra/physical/physical_ra.h"

namespace lra = fluent::ra::logical;
namespace pra = fluent::ra::physical;

namespace fluent {
namespace ra {

template <typename Logical>
auto LogicalToPhysical(const Logical& ra);

template <typename Logical>
struct LogicalToPhysicalImpl;

template <typename Physical>
auto Flatten(Physical p) {
  return pra::make_map(std::move(p), [](const auto& pair) {
    const auto& t = std::get<0>(pair);
    const std::set<LocalTupleId>& lineage = std::get<1>(pair);
    return tuple_cat(std::make_tuple(lineage), t);
  });
}

template <typename Physical>
auto UnFlatten(Physical p) {
  return pra::make_map(std::move(p), [](const auto& t) {
    const std::set<LocalTupleId>& lineage = std::get<0>(t);
    const auto t_ = common::TupleDrop<1>(t);
    return std::make_tuple(t_, lineage);
  });
}

template <typename Collection>
struct LogicalToPhysicalImpl<lra::Collection<Collection>> {
  auto operator()(const lra::Collection<Collection>& collection) {
    auto iterable = pra::make_iterable(&collection.collection->Get());
    return pra::make_map(std::move(iterable), [&collection](const auto& pair) {
      const auto& t = pair.first;
      const collections::CollectionTupleIds& ids = pair.second;
      const std::string& collection_name = collection.collection->Name();

      std::set<LocalTupleId> lineage;
      for (int logical_time_inserted : ids.logical_times_inserted) {
        LocalTupleId id{collection_name, ids.hash, logical_time_inserted};
        lineage.insert(id);
      }

      return std::make_tuple(t, lineage);
    });
  }
};

template <typename Collection>
struct LogicalToPhysicalImpl<lra::MetaCollection<Collection>> {
  auto operator()(const lra::MetaCollection<Collection>& meta_collection) {
    using column_types = typename lra::MetaCollection<Collection>::column_types;
    using column_tuple = typename common::TypeListToTuple<column_types>::type;
    using lineage_type = std::set<LocalTupleId>;
    using ret_type = std::vector<std::tuple<column_tuple, lineage_type>>;

    auto iterable = pra::make_iterable(&meta_collection.collection->Get());
    return pra::make_flat_map<ret_type>(
        std::move(iterable), [&meta_collection](const auto& pair) {
          const auto& t = pair.first;
          const collections::CollectionTupleIds& ids = pair.second;
          const Collection* collection = meta_collection.collection;
          const std::string& collection_name = collection->Name();

          ret_type ret;
          for (int logical_time_inserted : ids.logical_times_inserted) {
            LocalTupleId id{collection_name, ids.hash, logical_time_inserted};
            std::set<LocalTupleId> lineage{id};
            ret.push_back(std::make_tuple(std::make_tuple(t, id), lineage));
          }
          return ret;
        });
  }
};

template <typename Container>
struct LogicalToPhysicalImpl<lra::Iterable<Container>> {
  auto operator()(const lra::Iterable<Container>& iterable) const {
    auto iterable_ = pra::make_iterable(iterable.container);
    return pra::make_map(std::move(iterable_), [](const auto& t) {
      return std::make_tuple(t, std::set<LocalTupleId>{});
    });
  }
};

template <typename Logical, typename F>
struct LogicalToPhysicalImpl<lra::Map<Logical, F>> {
  auto operator()(const lra::Map<Logical, F>& map) {
    auto f = map.f;
    auto child = LogicalToPhysical(map.child);
    return pra::make_map(std::move(child), [f](const auto& pair) {
      const auto& t = std::get<0>(pair);
      const auto& lineage = std::get<1>(pair);
      return std::make_tuple(f(t), lineage);
    });
  }
};

template <typename Logical, typename F>
struct LogicalToPhysicalImpl<lra::Filter<Logical, F>> {
  auto operator()(const lra::Filter<Logical, F>& filter) {
    auto f = filter.f;
    auto child = LogicalToPhysical(filter.child);
    return pra::make_filter(std::move(child), [f](const auto& pair) {
      return f(std::get<0>(pair));
    });
  }
};

template <typename Logical, std::size_t... Is>
struct LogicalToPhysicalImpl<lra::Project<Logical, Is...>> {
  auto operator()(const lra::Project<Logical, Is...>& project) {
    auto child = Flatten(LogicalToPhysical(project.child));
    auto projected = pra::make_project<0, 1 + Is...>(std::move(child));
    return UnFlatten(std::move(projected));
  }
};

template <typename Left, typename Right>
struct LogicalToPhysicalImpl<lra::Cross<Left, Right>> {
  auto operator()(const lra::Cross<Left, Right>& cross) {
    auto left = LogicalToPhysical(cross.left);
    auto right = LogicalToPhysical(cross.right);
    auto cross_ = pra::make_cross(std::move(left), std::move(right));
    return pra::make_map(std::move(cross_), [](const auto& t) {
      const auto& left_t = std::get<0>(t);
      const std::set<LocalTupleId>& left_lineage = std::get<1>(t);
      const auto& right_t = std::get<2>(t);
      const std::set<LocalTupleId>& right_lineage = std::get<3>(t);

      std::set<LocalTupleId> lineage;
      lineage.insert(left_lineage.begin(), left_lineage.end());
      lineage.insert(right_lineage.begin(), right_lineage.end());
      return std::make_tuple(std::tuple_cat(left_t, right_t), lineage);
    });
  }
};

template <typename Left, std::size_t... LeftKs,  //
          typename Right, std::size_t... RightKs>
struct LogicalToPhysicalImpl<lra::HashJoin<Left, LeftKeys<LeftKs...>,  //
                                           Right, RightKeys<RightKs...>>> {
  auto operator()(
      const lra::HashJoin<Left, LeftKeys<LeftKs...>,  //
                          Right, RightKeys<RightKs...>>& hash_join) {
    using left_column_types = typename Left::column_types;
    using left_column_types_lineaged =
        typename common::TypeListCons<std::set<LocalTupleId>,
                                      left_column_types>::type;
    using left_column_tuple_lineaged =
        typename common::TypeListToTuple<left_column_types_lineaged>::type;
    static constexpr std::size_t left_num_columns =
        common::TypeListLen<left_column_types>::value;

    using left_key_column_types =
        typename common::TypeListProject<left_column_types, LeftKs...>::type;
    using left_key_column_tuple =
        typename common::TypeListToTuple<left_key_column_types>::type;

    auto left = Flatten(LogicalToPhysical(hash_join.left));
    auto right = Flatten(LogicalToPhysical(hash_join.right));
    using left_keys = LeftKeys<1 + LeftKs...>;
    using right_keys = RightKeys<1 + RightKs...>;

    auto joined =
        pra::make_hash_join<left_keys, right_keys, left_column_tuple_lineaged,
                            left_key_column_tuple>(std::move(left),
                                                   std::move(right));
    return pra::make_map(std::move(joined), [](const auto& t) {
      const std::set<LocalTupleId>& left_lineage = std::get<0>(t);
      const std::set<LocalTupleId>& right_lineage =
          std::get<1 + left_num_columns>(t);
      std::set<LocalTupleId> lineage;
      lineage.insert(left_lineage.begin(), left_lineage.end());
      lineage.insert(right_lineage.begin(), right_lineage.end());

      using left_indexes =
          typename common::SizetListRange<1, 1 + left_num_columns>::type;
      auto left_t = common::TupleProjectBySizetList<left_indexes>(t);
      auto right_t = common::TupleDrop<1 + left_num_columns + 1>(t);
      return std::make_tuple(std::tuple_cat(left_t, right_t), lineage);
    });
  }
};

template <typename AggregateImpl>
struct IncrementAggregateImpl;

template <template <typename, typename> class A,  //
          std::size_t... Is, typename... Ts>
struct IncrementAggregateImpl<
    A<common::SizetList<Is...>, common::TypeList<Ts...>>> {
  using aggregate_impl = A<common::SizetList<Is...>, common::TypeList<Ts...>>;
  using is_aggregate_impl = std::is_base_of<agg::AggregateImpl, aggregate_impl>;
  static_assert(common::StaticAssert<is_aggregate_impl>::value, "");
  using type = A<common::SizetList<1 + Is...>, common::TypeList<Ts...>>;
};

template <typename AggregateImpls>
struct IncrementAggregateImpls;

template <typename... AggImpls>
struct IncrementAggregateImpls<common::TypeList<AggImpls...>> {
  using type =
      common::TypeList<typename IncrementAggregateImpl<AggImpls>::type...>;
};

template <typename Logical, std::size_t... Ks, typename... Aggregates>
struct LogicalToPhysicalImpl<
    lra::GroupBy<Logical, Keys<Ks...>, Aggregates...>> {
  auto operator()(
      const lra::GroupBy<Logical, Keys<Ks...>, Aggregates...>& group_by) {
    using group = lra::GroupBy<Logical, Keys<Ks...>, Aggregates...>;

    using keys = Keys<1 + Ks...>;
    using key_types = typename group::key_types;
    using key_tuple = typename common::TypeListToTuple<key_types>::type;

    using agg_impl_types = typename group::aggregate_impl_types;
    using incr_agg_impl_types =
        typename IncrementAggregateImpls<agg_impl_types>::type;
    using union_ = agg::UnionImpl<common::SizetList<0>,
                                  common::TypeList<std::set<LocalTupleId>>>;
    using union_agg_impl_types =
        typename common::TypeListCons<union_, incr_agg_impl_types>::type;
    using agg_impl_tuple =
        typename common::TypeListToTuple<union_agg_impl_types>::type;

    auto child = Flatten(LogicalToPhysical(group_by.child));
    auto grouped =
        pra::make_group_by<keys, key_tuple, agg_impl_tuple>(std::move(child));
    return pra::make_map(std::move(grouped), [](const auto& t) {
      auto keys = common::TupleTake<sizeof...(Ks)>(t);
      auto lineage = std::get<sizeof...(Ks)>(t);
      auto aggs = common::TupleDrop<1 + sizeof...(Ks)>(t);
      return std::make_tuple(std::tuple_cat(keys, aggs), lineage);
    });
  }
};

template <typename Logical>
auto LogicalToPhysical(const Logical& l) {
  using logical_decayed = typename std::decay<Logical>::type;
  using is_logical = std::is_base_of<lra::LogicalRa, logical_decayed>;
  static_assert(common::StaticAssert<is_logical>::value, "");

  auto p = LogicalToPhysicalImpl<logical_decayed>()(l);

  using physical = decltype(p);
  using is_physical = std::is_base_of<pra::PhysicalRa, physical>;
  static_assert(common::StaticAssert<is_physical>::value, "");

  return std::move(p);
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_LOGICAL_TO_PHYSICAL_H_
