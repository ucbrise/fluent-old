#ifndef RA_PHYSICAL_MAP_H_
#define RA_PHYSICAL_MAP_H_

#include <type_traits>

#include "range/v3/all.hpp"

#include "common/macros.h"
#include "common/static_assert.h"
#include "ra/physical/physical_ra.h"

namespace fluent {
namespace ra {
namespace physical {

template <typename Ra, typename F>
class Map : public PhysicalRa {
  static_assert(common::StaticAssert<std::is_base_of<PhysicalRa, Ra>>::value,
                "");

 public:
  Map(Ra child, F f) : child_(std::move(child)), f_(std::move(f)) {}
  DISALLOW_COPY_AND_ASSIGN(Map);
  DEFAULT_MOVE_AND_ASSIGN(Map);

  auto ToRange() { return child_.ToRange() | ranges::view::transform(f_); }

 private:
  Ra child_;
  F f_;
};

template <typename Ra, typename F,
          typename RaDecayed = typename std::decay<Ra>::type,
          typename FDecayed = typename std::decay<F>::type>

Map<RaDecayed, FDecayed> make_map(Ra&& child, F&& f) {
  return Map<RaDecayed, FDecayed>(std::forward<Ra>(child), std::forward<F>(f));
}

}  // namespace physical
}  // namespace ra
}  // namespace fluent

#endif  // RA_PHYSICAL_MAP_H_
