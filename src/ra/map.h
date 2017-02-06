#ifndef RA_MAP_H_
#define RA_MAP_H_

#include <utility>

#include "range/v3/all.hpp"

namespace fluent {
namespace ra {

template <typename T, typename F>
class Map {
 public:
  Map(T child, F f) : child_(std::move(child)), f_(std::move(f)) {}

  auto ToRange() const {
    return child_.ToRange() | ranges::view::transform(f_);
  }

 private:
  const T child_;
  F f_;
};

template <typename T, typename F>
Map<T, F> make_map(T&& child, F&& f) {
  return Map<T, F>(std::forward<T>(child), std::forward<F>(f));
}

template <typename F>
struct MapPipe {
  F f;
};

template <typename F>
MapPipe<F> map(F&& f) {
  return {std::forward<F>(f)};
}

template <typename T, typename F>
Map<T, F> operator|(T&& child, MapPipe<F> f) {
  return make_map(std::forward<T&&>(child), std::move(f.f));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_MAP_H_
