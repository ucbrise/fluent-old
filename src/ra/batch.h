#ifndef RA_BATCH_H_
#define RA_BATCH_H_

#include <cstddef>

#include <set>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/type_list.h"

namespace fluent {
namespace ra {

template <typename PhysicalChild, typename LogicalBatch>
class PhysicalBatch {
 public:
  explicit PhysicalBatch(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    typename LogicalBatch::column_types::template type<0> s;
    auto rng = child_.ToRange();
    for (auto iter = ranges::begin(rng); iter != ranges::end(rng); ++iter) {
      s.insert(*iter);
    }
    batch_.insert(std::make_tuple(s));
    return ranges::view::all(batch_);
  }

 private:
  PhysicalChild child_;
  std::set<typename TypeListToTuple<typename LogicalBatch::column_types>::type> batch_;
};

template <typename LogicalChild>
class Batch {
 public:
  using column_types = TypeList<std::set<typename TypeListToTuple<typename LogicalChild::column_types>::type>>;

  Batch(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const {
    return PhysicalBatch<
        decltype(child_.ToPhysical()),
        Batch<LogicalChild>>(
        child_.ToPhysical());
  }

 private:
  const LogicalChild child_;
};

template <typename LogicalChild>
Batch<typename std::decay<LogicalChild>::type> make_batch(
    LogicalChild&& child) {
  return {std::forward<LogicalChild>(child)};
}

struct BatchPipe {};

BatchPipe batch() { return {}; }

template <typename LogicalChild>
Batch<typename std::decay<LogicalChild>::type> operator|(LogicalChild&& child,
                                                         BatchPipe) {
  return make_batch(std::forward<LogicalChild>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_BATCH_H_
