#ifndef RA_HASH_JOIN_H_
#define RA_HASH_JOIN_H_

#include <cstddef>
#include <map>
#include <type_traits>
#include <utility>

#include "fmt/format.h"
#include "range/v3/all.hpp"

#include "common/string_util.h"
#include "common/tuple_util.h"
#include "common/type_list.h"
#include "ra/lineaged_tuple.h"

namespace fluent {
namespace ra {

template <std::size_t... Ks>
struct LeftKeys {};

template <std::size_t... Ks>
struct RightKeys {};

template <typename PhysicalLeft, typename LeftKeys, typename PhysicalRight,
          typename RightKeys, typename LogicalHashJoin>
class PhysicalHashJoin;

template <typename PhysicalLeft, std::size_t... LeftKs, typename PhysicalRight,
          std::size_t... RightKs, typename LogicalHashJoin>
class PhysicalHashJoin<PhysicalLeft, LeftKeys<LeftKs...>, PhysicalRight,
                       RightKeys<RightKs...>, LogicalHashJoin> {
 public:
  using left_column_types = typename LogicalHashJoin::left_column_types;
  using left_column_tuple = typename TypeListToTuple<left_column_types>::type;
  using left_key_column_types =
      typename TypeListProject<left_column_types, LeftKs...>::type;
  using left_key_column_tuple =
      typename TypeListToTuple<left_key_column_types>::type;
  using left_column_lineaged_tuple =
      typename TypeListTo<LineagedTuple,
                          typename LogicalHashJoin::left_column_types>::type;

  PhysicalHashJoin(PhysicalLeft left, PhysicalRight right)
      : left_(std::move(left)), right_(std::move(right)) {}

  auto ToRange() {
    if (left_hash_.size() == 0) {
      ranges::for_each(left_.ToRange(), [this](const auto& lt) {
        left_hash_[TupleProject<LeftKs...>(lt.tuple)].push_back(lt);
      });
    }

    return ranges::view::for_each(right_.ToRange(), [this](const auto& right) {
      return ranges::yield_from(
          ranges::view::all(left_hash_[TupleProject<RightKs...>(right.tuple)]) |
          ranges::view::transform([right](auto left) {
            left.lineage.insert(right.lineage.begin(), right.lineage.end());
            return make_lineaged_tuple(
                std::move(left.lineage),
                std::tuple_cat(std::move(left.tuple), right.tuple));
          }));
    });
  }

 private:
  PhysicalLeft left_;
  PhysicalRight right_;
  std::map<left_key_column_tuple, std::vector<left_column_lineaged_tuple>>
      left_hash_;
};

template <typename LogicalLeft, typename LeftKeys, typename LogicalRight,
          typename RightKeys>
class HashJoin;

template <typename LogicalLeft, std::size_t... LeftKs, typename LogicalRight,
          std::size_t... RightKs>
class HashJoin<LogicalLeft, LeftKeys<LeftKs...>, LogicalRight,
               RightKeys<RightKs...>> {
 public:
  using left_column_types = typename LogicalLeft::column_types;
  using right_column_types = typename LogicalRight::column_types;
  using column_types =
      typename TypeListConcat<left_column_types, right_column_types>::type;

  HashJoin(LogicalLeft left, LogicalRight right)
      : left_(std::move(left)), right_(std::move(right)) {}

  auto ToPhysical() const {
    return PhysicalHashJoin<decltype(left_.ToPhysical()), LeftKeys<LeftKs...>,
                            decltype(right_.ToPhysical()),
                            RightKeys<RightKs...>,
                            HashJoin<LogicalLeft, LeftKeys<LeftKs...>,
                                     LogicalRight, RightKeys<RightKs...>>>(
        left_.ToPhysical(), right_.ToPhysical());
  }

  std::string ToDebugString() const {
    return fmt::format("HashJoin<Left<{}>, Right<{}>>({}, {})", Join(LeftKs...),
                       Join(RightKs...), left_.ToDebugString(),
                       right_.ToDebugString());
  }

 private:
  const LogicalLeft left_;
  const LogicalRight right_;
};

template <typename LeftKeys, typename RightKeys, typename LogicalLeft,
          typename LogicalRight>
HashJoin<typename std::decay<LogicalLeft>::type, LeftKeys,
         typename std::decay<LogicalRight>::type, RightKeys>
make_hash_join(LogicalLeft&& left, LogicalRight&& right) {
  return {std::forward<LogicalLeft>(left), std::forward<LogicalRight>(right)};
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_HASH_JOIN_H_
