#ifndef RA_SUM_H_
#define RA_SUM_H_

#include <cstddef>

#include <set>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/type_list.h"

namespace fluent {
namespace ra {

template <typename PhysicalChild, typename LogicalSum>
class PhysicalSum {
 public:
  using child_column_types = typename LogicalSum::column_types;
  using child_column_type = typename child_column_types::template type<0>;
  using child_column_tuple = typename TypeListToTuple<child_column_types>::type;

  explicit PhysicalSum(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    child_column_type s = 0;
    auto rng = child_.ToRange();
    for (auto iter = ranges::begin(rng); iter != ranges::end(rng); ++iter) {
      s += std::get<0>(*iter);
    }
    sum_.insert(child_column_tuple(s));
    return ranges::view::all(sum_);
  }

 private:
  PhysicalChild child_;
  std::set<child_column_tuple> sum_;
};

template <typename LogicalChild>
class Sum {
 public:
  using column_types = typename LogicalChild::column_types;

  Sum(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const{
    return PhysicalSum<
        decltype(child_.ToPhysical()),
        Sum<LogicalChild>>(
        child_.ToPhysical());
  }

 private:
  const LogicalChild child_;
};

template <typename LogicalChild>
Sum<typename std::decay<LogicalChild>::type> make_sum(
    LogicalChild&& child) {
  return {std::forward<LogicalChild>(child)};
}

struct SumPipe {};

SumPipe sum() { return {}; }

template <typename LogicalChild>
Sum<typename std::decay<LogicalChild>::type> operator|(LogicalChild&& child,
                                                         SumPipe) {
  return make_sum(std::forward<LogicalChild>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_SUM_H_
