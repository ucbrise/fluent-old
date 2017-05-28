#ifndef RA_LOGICAL_CROSS_H_
#define RA_LOGICAL_CROSS_H_

#include <type_traits>

#include "common/static_assert.h"
#include "common/type_list.h"
#include "common/type_traits.h"
#include "ra/logical/logical_ra.h"

namespace fluent {
namespace ra {
namespace logical {

template <typename Left, typename Right>
struct Cross : public LogicalRa {
  static_assert(StaticAssert<std::is_base_of<LogicalRa, Left>>::value, "");
  static_assert(StaticAssert<std::is_base_of<LogicalRa, Right>>::value, "");
  using left_column_types = typename Left::column_types;
  using right_column_types = typename Right::column_types;

  using column_types =
      typename TypeListConcat<left_column_types, right_column_types>::type;
  Cross(Left left_, Right right_)
      : left(std::move(left_)), right(std::move(right_)) {}
  Left left;
  Right right;
};

template <typename Left, typename Right,
          typename LeftDecayed = typename std::decay<Left>::type,
          typename RightDecayed = typename std::decay<Right>::type>
Cross<LeftDecayed, RightDecayed> make_cross(Left&& left, Right&& right) {
  return Cross<LeftDecayed, RightDecayed>{std::forward<Left>(left),
                                          std::forward<Right>(right)};
}

}  // namespace logical
}  // namespace ra
}  // namespace fluent

#endif  // RA_LOGICAL_CROSS_H_
