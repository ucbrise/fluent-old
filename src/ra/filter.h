#ifndef RA_FILTER_H_
#define RA_FILTER_H_

#include <utility>

#include "range/v3/all.hpp"

namespace fluent {
namespace ra {

template <typename T, typename F>
class Filter {
 public:
  Filter(T child, F f) : child_(std::move(child)), f_(std::move(f)) {}

  auto ToRange() const { return child_.ToRange() | ranges::view::filter(f_); }

 private:
  const T child_;
  F f_;
};

template <typename T, typename F>
Filter<T, F> make_filter(T&& child, F&& f) {
  return Filter<T, F>(std::forward<T>(child), std::forward<F>(f));
}

template <typename F>
struct FilterPipe {
  F f;
};

template <typename F>
FilterPipe<F> filter(F&& f) {
  return {std::forward<F>(f)};
}

template <typename T, typename F>
Filter<T, F> operator|(T&& child, FilterPipe<F> f) {
  return make_filter(std::forward<T&&>(child), std::move(f.f));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_FILTER_H_
