#ifndef RA_PHYSICAL_FLAT_MAP_H_
#define RA_PHYSICAL_FLAT_MAP_H_

#include <type_traits>

#include "range/v3/all.hpp"

#include "common/macros.h"
#include "common/static_assert.h"
#include "ra/physical/physical_ra.h"

namespace fluent {
namespace ra {
namespace physical {

template <typename Ra, typename F, typename Ret>
class FlatMap : public PhysicalRa {
  static_assert(common::StaticAssert<std::is_base_of<PhysicalRa, Ra>>::value,
                "");

 public:
  FlatMap(Ra child, F f) : child_(std::move(child)), f_(std::move(f)) {}
  DISALLOW_COPY_AND_ASSIGN(FlatMap);
  DEFAULT_MOVE_AND_ASSIGN(FlatMap);

  auto ToRange() {
    return child_.ToRange()  //
           | ranges::view::for_each([this](const auto& t) {
               ret_ = f_(t);
               return ranges::yield_from(ranges::view::all(ret_));
             });
  }

 private:
  Ra child_;
  F f_;
  Ret ret_;
};

template <typename Ret, typename Ra, typename F,
          typename RaDecayed = typename std::decay<Ra>::type,
          typename FDecayed = typename std::decay<F>::type>

FlatMap<RaDecayed, FDecayed, Ret> make_flat_map(Ra&& child, F&& f) {
  return FlatMap<RaDecayed, FDecayed, Ret>(std::forward<Ra>(child),
                                           std::forward<F>(f));
}

}  // namespace physical
}  // namespace ra
}  // namespace fluent

#endif  // RA_PHYSICAL_FLAT_MAP_H_
