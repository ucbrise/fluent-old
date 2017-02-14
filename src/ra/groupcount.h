#ifndef RA_GroupCount_H_
#define RA_GroupCount_H_

#include <cstddef>

#include <set>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

namespace fluent {
namespace ra {

template <typename PhysicalChild>
class PhysicalGroupCount {
 public:
  explicit PhysicalGroupCount(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    return child_.ToRange() | ranges::view::transform([](const auto& rng) {
          return std::make_tuple(ranges::size(rng));
       });
  }

 private:
  PhysicalChild child_;
};

template <typename Physical>
PhysicalGroupCount<typename std::decay<Physical>::type> make_physical_groupcount(
    Physical&& child) {
  return PhysicalGroupCount<typename std::decay<Physical>::type>(
      std::forward<Physical>(child));
}

template <typename LogicalChild>
class GroupCount {
 public:
  GroupCount(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const { return make_physical_groupcount(child_.ToPhysical()); }

 private:
  const LogicalChild child_;
};

template <typename LogicalChild>
GroupCount<typename std::decay<LogicalChild>::type> make_groupcount(
    LogicalChild&& child) {
  return {std::forward<LogicalChild>(child)};
}

struct GroupCountPipe {};

GroupCountPipe groupcount() { return {}; }

template <typename LogicalChild>
GroupCount<typename std::decay<LogicalChild>::type> operator|(LogicalChild&& child,
                                                         GroupCountPipe) {
  return make_groupcount(std::forward<LogicalChild>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_GroupCount_H_
