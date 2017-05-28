#ifndef RA_LOGICAL_HASH_JOIN_H_
#define RA_LOGICAL_HASH_JOIN_H_

#include <cstddef>

#include <type_traits>

#include "common/static_assert.h"
#include "common/type_list.h"
#include "common/type_traits.h"
#include "ra/keys.h"
#include "ra/logical/logical_ra.h"

namespace fluent {
namespace ra {
namespace logical {

template <typename Left, typename LeftKeys, typename Right, typename RightKeys>
struct HashJoin;

template <typename Left, std::size_t... LeftKs, typename Right,
          std::size_t... RightKs>
struct HashJoin<Left, LeftKeys<LeftKs...>, Right, RightKeys<RightKs...>>
    : public LogicalRa {
  static_assert(StaticAssert<std::is_base_of<LogicalRa, Left>>::value, "");
  static_assert(StaticAssert<std::is_base_of<LogicalRa, Right>>::value, "");
  using left_size = std::integral_constant<std::size_t, sizeof...(LeftKs)>;
  using right_size = std::integral_constant<std::size_t, sizeof...(RightKs)>;
  static_assert(StaticAssert<std::is_same<left_size, right_size>>::value, "");
  using left_column_types = typename Left::column_types;
  using right_column_types = typename Right::column_types;

  using column_types =
      typename TypeListConcat<left_column_types, right_column_types>::type;
  HashJoin(Left left_, Right right_)
      : left(std::move(left_)), right(std::move(right_)) {}
  Left left;
  Right right;
};

template <typename LeftKeys, typename RightKeys, typename Left, typename Right,
          typename LeftDecayed = typename std::decay<Left>::type,
          typename RightDecayed = typename std::decay<Right>::type>
HashJoin<LeftDecayed, LeftKeys, RightDecayed, RightKeys> make_hash_join(
    Left&& left, Right&& right) {
  return HashJoin<LeftDecayed, LeftKeys, RightDecayed, RightKeys>(
      std::forward<Left>(left), std::forward<Right>(right));
}

}  // namespace logical
}  // namespace ra
}  // namespace fluent

#endif  // RA_LOGICAL_HASH_JOIN_H_
