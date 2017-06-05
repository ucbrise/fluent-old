#ifndef RA_FILTER_H_
#define RA_FILTER_H_

#include <type_traits>
#include <utility>

#include "fmt/format.h"
#include "range/v3/all.hpp"

#include "common/hash_util.h"
#include "ra/lineaged_tuple.h"

namespace fluent {
namespace ra {

template <typename PhysicalChild, typename F>
class PhysicalFilter {
 public:
  PhysicalFilter(PhysicalChild child, F* f) : child_(std::move(child)), f_(f) {}

  auto ToRange() {
    return child_.ToRange() | ranges::view::filter([this](const auto& lt) {
             return (*f_)(lt.tuple);
           });
  }

 private:
  PhysicalChild child_;
  F* f_;
};

template <typename PhysicalChild, typename F>
PhysicalFilter<typename std::decay<PhysicalChild>::type, F>
make_physical_filter(PhysicalChild&& child, F* f) {
  return {std::forward<PhysicalChild>(child), f};
}

template <typename LogicalChild, typename F>
class Filter {
 public:
  using column_types = typename LogicalChild::column_types;

  Filter(LogicalChild child, F f)
      : child_(std::move(child)), f_(std::move(f)) {}

  auto ToPhysical() const {
    return make_physical_filter(child_.ToPhysical(), &f_);
  }

  std::string ToDebugString() const {
    return fmt::format("Filter({})", child_.ToDebugString());
  }

 private:
  const LogicalChild child_;
  F f_;
};

template <typename LogicalChild, typename F>
Filter<typename std::decay<LogicalChild>::type, typename std::decay<F>::type>
make_filter(LogicalChild&& child, F&& f) {
  return {std::forward<LogicalChild>(child), std::forward<F>(f)};
}

template <typename F>
struct FilterPipe {
  F f;
};

template <typename F>
FilterPipe<typename std::decay<F>::type> filter(F&& f) {
  return {std::forward<F>(f)};
}

template <typename LogicalChild, typename F>
Filter<typename std::decay<LogicalChild>::type, F> operator|(
    LogicalChild&& child, FilterPipe<F> f) {
  return make_filter(std::forward<LogicalChild&&>(child), std::move(f.f));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_FILTER_H_
