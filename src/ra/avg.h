#ifndef RA_AVG_H_
#define RA_AVG_H_

#include <cstddef>

#include <set>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/type_list.h"

namespace fluent {
namespace ra {

template <typename PhysicalChild, typename LogicalAvg>
class PhysicalAvg {
 public:
  using child_column_types = typename LogicalAvg::column_types;
  using child_column_type = typename child_column_types::template type<0>;
  using child_column_tuple = typename TypeListToTuple<child_column_types>::type;

  explicit PhysicalAvg(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    child_column_type s = 0;
    std::size_t c = 0;
    auto rng = child_.ToRange();
    for (auto iter = ranges::begin(rng); iter != ranges::end(rng); ++iter) {
      s += std::get<0>(*iter);
      c ++;
    }
    if (c == 0) avg_.insert(child_column_tuple(0));
    else avg_.insert(child_column_tuple(s/c));
    return ranges::view::all(avg_);
  }

 private:
  PhysicalChild child_;
  std::set<child_column_tuple> avg_;
};

template <typename LogicalChild>
class Avg {
 public:
  using column_types = typename LogicalChild::column_types;

  Avg(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const{
    return PhysicalAvg<
        decltype(child_.ToPhysical()),
        Avg<LogicalChild>>(
        child_.ToPhysical());
  }

 private:
  const LogicalChild child_;
};

template <typename LogicalChild>
Avg<typename std::decay<LogicalChild>::type> make_avg(
    LogicalChild&& child) {
  return {std::forward<LogicalChild>(child)};
}

struct AvgPipe {};

AvgPipe avg() { return {}; }

template <typename LogicalChild>
Avg<typename std::decay<LogicalChild>::type> operator|(LogicalChild&& child,
                                                         AvgPipe) {
  return make_avg(std::forward<LogicalChild>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_AVG_H_
