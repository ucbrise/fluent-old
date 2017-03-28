#ifndef RA_COUNT_H_
#define RA_COUNT_H_

#include <cstddef>

#include <set>
#include <type_traits>
#include <utility>

#include "fmt/format.h"
#include "range/v3/all.hpp"

#include "common/type_list.h"

namespace fluent {
namespace ra {

template <typename PhysicalChild>
class PhysicalCount {
 public:
  explicit PhysicalCount(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    if (count_.size() == 0) {
      count_.insert(std::tuple<std::size_t>(ranges::size(child_.ToRange())));
    }
    return ranges::view::all(count_);
  }

 private:
  PhysicalChild child_;
  std::set<std::tuple<std::size_t>> count_;
};

template <typename Physical>
PhysicalCount<typename std::decay<Physical>::type> make_physical_count(
    Physical&& child) {
  return PhysicalCount<typename std::decay<Physical>::type>(
      std::forward<Physical>(child));
}

template <typename LogicalChild>
class Count {
 public:
  using column_types = TypeList<std::size_t>;

  Count(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const { return make_physical_count(child_.ToPhysical()); }

  std::string ToDebugString() const {
    return fmt::format("Count({})", child_.ToDebugString());
  }

 private:
  const LogicalChild child_;
};

template <typename LogicalChild>
Count<typename std::decay<LogicalChild>::type> make_count(
    LogicalChild&& child) {
  return {std::forward<LogicalChild>(child)};
}

struct CountPipe {};

CountPipe count() { return {}; }

template <typename LogicalChild>
Count<typename std::decay<LogicalChild>::type> operator|(LogicalChild&& child,
                                                         CountPipe) {
  return make_count(std::forward<LogicalChild>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_COUNT_H_
