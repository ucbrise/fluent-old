#ifndef RA_PHYSICAL_FILTER_H_
#define RA_PHYSICAL_FILTER_H_

#include <type_traits>

#include "range/v3/all.hpp"

#include "common/macros.h"
#include "common/static_assert.h"
#include "ra/physical/physical_ra.h"

namespace fluent {
namespace ra {
namespace physical {

template <typename Ra, typename F>
class Filter : public PhysicalRa {
  static_assert(StaticAssert<std::is_base_of<PhysicalRa, Ra>>::value, "");

 public:
  Filter(Ra child, F f) : child_(std::move(child)), f_(std::move(f)) {}
  DISALLOW_COPY_AND_ASSIGN(Filter);
  DEFAULT_MOVE_AND_ASSIGN(Filter);

  auto ToRange() { return child_.ToRange() | ranges::view::filter(f_); }

 private:
  Ra child_;
  F f_;
};

template <typename Ra, typename F,
          typename RaDecayed = typename std::decay<Ra>::type,
          typename FDecayed = typename std::decay<F>::type>
Filter<RaDecayed, FDecayed> make_filter(Ra&& child, F&& f) {
  return Filter<RaDecayed, FDecayed>(std::forward<Ra>(child),
                                     std::forward<F>(f));
}

}  // namespace physical
}  // namespace ra
}  // namespace fluent

#endif  // RA_PHYSICAL_FILTER_H_
