#ifndef RA_PHYSICAL_CROSS_H_
#define RA_PHYSICAL_CROSS_H_

#include <tuple>
#include <type_traits>

#include "range/v3/all.hpp"

#include "common/macros.h"
#include "common/static_assert.h"
#include "ra/physical/physical_ra.h"

namespace fluent {
namespace ra {
namespace physical {

template <typename Left, typename Right>
class Cross : public PhysicalRa {
  static_assert(StaticAssert<std::is_base_of<PhysicalRa, Left>>::value, "");
  static_assert(StaticAssert<std::is_base_of<PhysicalRa, Right>>::value, "");

 public:
  Cross(Left left, Right right)
      : left_(std::move(left)), right_(std::move(right)) {}
  DISALLOW_COPY_AND_ASSIGN(Cross);
  DEFAULT_MOVE_AND_ASSIGN(Cross);

  auto ToRange() {
    return ranges::view::cartesian_product(left_.ToRange(), right_.ToRange()) |
           ranges::view::transform([](const auto& t) {
             return std::tuple_cat(std::get<0>(t), std::get<1>(t));
           });
  }

 private:
  Left left_;
  Right right_;
};

template <typename Left, typename Right,
          typename LeftDecayed = typename std::decay<Left>::type,
          typename RightDecayed = typename std::decay<Right>::type>
Cross<LeftDecayed, RightDecayed> make_cross(Left&& left, Right&& right) {
  return Cross<LeftDecayed, RightDecayed>{std::forward<Left>(left),
                                          std::forward<Right>(right)};
}

}  // namespace physical
}  // namespace ra
}  // namespace fluent

#endif  // RA_PHYSICAL_CROSS_H_
