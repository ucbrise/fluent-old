#ifndef RA_PHYSICAL_GROUP_BY_H_
#define RA_PHYSICAL_GROUP_BY_H_

#include <type_traits>
#include <map>

#include "range/v3/all.hpp"

#include "common/macros.h"
#include "common/static_assert.h"
#include "common/tuple_util.h"
#include "common/type_traits.h"
#include "ra/keys.h"
#include "ra/physical/physical_ra.h"

namespace fluent {
namespace ra {
namespace physical {

template <typename Ra, typename Keys, typename KeyColumnTuple,
          typename AggregateImplTuple>
class GroupBy;

template <typename Ra, std::size_t... Ks, typename KeyColumnTuple,
          typename AggregateImplTuple>
class GroupBy<Ra, Keys<Ks...>, KeyColumnTuple, AggregateImplTuple>
    : public PhysicalRa {
  static_assert(StaticAssert<std::is_base_of<PhysicalRa, Ra>>::value, "");
  static_assert(StaticAssert<IsTuple<AggregateImplTuple>>::value, "");

 public:
  explicit GroupBy(Ra child) : child_(std::move(child)) {}
  DISALLOW_COPY_AND_ASSIGN(GroupBy);
  DEFAULT_MOVE_AND_ASSIGN(GroupBy);

  auto ToRange() {
    groups_.clear();

    ranges::for_each(child_.ToRange(), [this](const auto& t) {
      auto& group = groups_[TupleProject<Ks...>(t)];
      TupleIter(group, [this, &t](auto& agg) { this->UpdateAgg(&agg, t); });
    });

    return ranges::view::all(groups_) |
           ranges::view::transform([](const auto& pair) {
             const auto& keys = pair.first;
             auto groups = TupleMap(pair.second,
                                    [](const auto& agg) { return agg.Get(); });
             return std::tuple_cat(keys, std::move(groups));
           });
  }

 private:
  template <template <typename, typename> class AggregateImpl,  //
            typename Columns, typename Ts, typename... Us>
  void UpdateAgg(AggregateImpl<Columns, Ts>* agg, const std::tuple<Us...>& t) {
    agg->Update(TupleProjectBySizetList<Columns>(t));
  }

  Ra child_;
  std::map<KeyColumnTuple, AggregateImplTuple> groups_;
};

template <typename Keys, typename KeyColumnTuple, typename AggregateImplTuple,
          typename Ra, typename RaDecayed = typename std::decay<Ra>::type>
GroupBy<RaDecayed, Keys, KeyColumnTuple, AggregateImplTuple> make_group_by(
    Ra&& ra) {
  return GroupBy<RaDecayed, Keys, KeyColumnTuple, AggregateImplTuple>(
      std::forward<Ra>(ra));
}

}  // namespace physical
}  // namespace ra
}  // namespace fluent

#endif  // RA_PHYSICAL_GROUP_BY_H_
