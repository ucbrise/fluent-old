#ifndef RA_CROSS_H_
#define RA_CROSS_H_

#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

namespace fluent {
namespace ra {

template <typename PhysicalLeft, typename PhysicalRight>
class PhysicalCross {
 public:
  PhysicalCross(PhysicalLeft left, PhysicalRight right)
      : left_(std::move(left)), right_(std::move(right)) {}

  auto ToRange() {
    return ranges::view::for_each(left_.ToRange(), [this](const auto& left) {
      return ranges::yield_from(
          right_.ToRange() | ranges::view::transform([left](const auto& right) {
            return std::tuple_cat(left, right);
          }));
    });
  }

 private:
  PhysicalLeft left_;
  PhysicalRight right_;
};

template <typename PhysicalLeft, typename PhysicalRight>
PhysicalCross<typename std::decay<PhysicalLeft>::type,
              typename std::decay<PhysicalRight>::type>
make_physical_cross(PhysicalLeft&& left, PhysicalRight&& right) {
  return {std::forward<PhysicalLeft>(left), std::forward<PhysicalRight>(right)};
}

template <typename LogicalLeft, typename LogicalRight>
class Cross {
 public:
  Cross(LogicalLeft left, LogicalRight right)
      : left_(std::move(left)), right_(std::move(right)) {}

  auto ToPhysical() const {
    return make_physical_cross(left_.ToPhysical(), right_.ToPhysical());
  }

 private:
  const LogicalLeft left_;
  const LogicalRight right_;
};

template <typename LogicalLeft, typename LogicalRight>
Cross<typename std::decay<LogicalLeft>::type,
      typename std::decay<LogicalRight>::type>
make_cross(LogicalLeft&& left, LogicalRight&& right) {
  return {std::forward<LogicalLeft>(left), std::forward<LogicalRight>(right)};
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_CROSS_H_
