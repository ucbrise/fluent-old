#ifndef RA_MAP_H_
#define RA_MAP_H_

#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

namespace fluent {
namespace ra {

template <typename PhysicalChild, typename F>
class PhysicalMap {
 public:
  PhysicalMap(PhysicalChild child, F* f) : child_(child), f_(f) {}

  auto ToRange() {
    return child_.ToRange() | ranges::view::transform(*f_);
  }

 private:
  PhysicalChild child_;
  F* f_;
};

template <typename PhysicalChild, typename F>
PhysicalMap<typename std::decay<PhysicalChild>::type, F> make_physical_map(
    PhysicalChild&& child, F* f) {
  return {std::forward<PhysicalChild>(child), f};
}

template <typename LogicalChild, typename F>
class Map {
 public:
  Map(LogicalChild child, F f) : child_(std::move(child)), f_(std::move(f)) {}

  auto ToPhysical() const {
    return make_physical_map(child_.ToPhysical(), &f_);
  }

 private:
  const LogicalChild child_;
  F f_;
};

template <typename LogicalChild, typename F>
Map<typename std::decay<LogicalChild>::type, typename std::decay<F>::type>
make_map(LogicalChild&& child, F&& f) {
  return {std::forward<LogicalChild>(child), std::forward<F>(f)};
}

template <typename F>
struct MapPipe {
  F f;
};

template <typename F>
MapPipe<typename std::decay<F>::type> map(F&& f) {
  return {std::forward<F>(f)};
}

template <typename LogicalChild, typename F>
Map<typename std::decay<LogicalChild>::type, F> operator|(LogicalChild&& child,
                                                          MapPipe<F> f) {
  return make_map(std::forward<LogicalChild>(child), std::move(f.f));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_MAP_H_
