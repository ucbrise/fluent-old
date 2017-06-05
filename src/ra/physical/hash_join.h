#ifndef RA_PHYSICAL_HASH_JOIN_H_
#define RA_PHYSICAL_HASH_JOIN_H_

#include <map>
#include <type_traits>
#include <vector>

#include "range/v3/all.hpp"

#include "common/macros.h"
#include "common/static_assert.h"
#include "common/tuple_util.h"
#include "ra/keys.h"
#include "ra/physical/physical_ra.h"

namespace fluent {
namespace ra {
namespace physical {

template <typename Left, typename LeftKeys, typename Right, typename RightKeys,
          typename LeftColumnTuple, typename LeftKeyColumnTuple>
class HashJoin;

template <typename Left, std::size_t... LeftKs, typename Right,
          std::size_t... RightKs, typename LeftColumnTuple,
          typename LeftKeyColumnTuple>
class HashJoin<Left, LeftKeys<LeftKs...>, Right, RightKeys<RightKs...>,
               LeftColumnTuple, LeftKeyColumnTuple> : public PhysicalRa {
  static_assert(StaticAssert<std::is_base_of<PhysicalRa, Left>>::value, "");
  static_assert(StaticAssert<std::is_base_of<PhysicalRa, Right>>::value, "");
  static_assert(StaticAssert<IsTuple<LeftColumnTuple>>::value, "");
  static_assert(StaticAssert<IsTuple<LeftKeyColumnTuple>>::value, "");

 public:
  HashJoin(Left left, Right right)
      : left_(std::move(left)), right_(std::move(right)) {}
  DISALLOW_COPY_AND_ASSIGN(HashJoin);
  DEFAULT_MOVE_AND_ASSIGN(HashJoin);

  auto ToRange() {
    left_hash_.clear();

    ranges::for_each(left_.ToRange(), [this](const auto& t) {
      const auto keys = TupleProject<LeftKs...>(t);
      using column = typename std::decay<decltype(t)>::type;
      using key = typename std::decay<decltype(keys)>::type;
      using column_matches = std::is_same<LeftColumnTuple, column>;
      using key_matches = std::is_same<LeftKeyColumnTuple, key>;
      static_assert(StaticAssert<column_matches>::value, "");
      static_assert(StaticAssert<key_matches>::value, "");
      left_hash_[std::move(keys)].push_back(t);
    });

    return ranges::view::for_each(right_.ToRange(), [this](const auto& right) {
      return ranges::yield_from(
          ranges::view::all(left_hash_[TupleProject<RightKs...>(right)]) |
          ranges::view::transform([right](const auto& left) {
            return std::tuple_cat(left, right);
          }));
    });
  }

 private:
  Left left_;
  Right right_;
  std::map<LeftKeyColumnTuple, std::vector<LeftColumnTuple>> left_hash_;
};

template <typename LeftKeys, typename RightKeys, typename LeftColumnTuple,
          typename LeftKeyColumnTuple, typename Left, typename Right,
          typename LeftDecayed = typename std::decay<Left>::type,
          typename RightDecayed = typename std::decay<Right>::type>
HashJoin<LeftDecayed, LeftKeys, RightDecayed, RightKeys, LeftColumnTuple,
         LeftKeyColumnTuple>
make_hash_join(Left&& left, Right&& right) {
  return HashJoin<LeftDecayed, LeftKeys, RightDecayed, RightKeys,
                  LeftColumnTuple, LeftKeyColumnTuple>(
      std::forward<Left>(left), std::forward<Right>(right));
}

}  // namespace physical
}  // namespace ra
}  // namespace fluent

#endif  // RA_PHYSICAL_HASH_JOIN_H_
