#ifndef RA_LOGICAL_MAP_H_
#define RA_LOGICAL_MAP_H_

#include <type_traits>

#include "common/static_assert.h"
#include "common/type_list.h"
#include "common/type_traits.h"
#include "ra/logical/logical_ra.h"

namespace fluent {
namespace ra {
namespace logical {

template <typename Ra, typename F>
struct Map : public LogicalRa {
  static_assert(StaticAssert<std::is_base_of<LogicalRa, Ra>>::value, "");
  using child_column_types = typename Ra::column_types;
  using child_column_tuple = typename TypeListToTuple<child_column_types>::type;
  static_assert(StaticAssert<IsInvocable<F, child_column_tuple>>::value, "");
  using column_tuple = typename std::result_of<F(child_column_tuple)>::type;
  using column_tuple_decayed = typename std::decay<column_tuple>::type;
  static_assert(StaticAssert<IsTuple<column_tuple_decayed>>::value, "");

  using column_types = typename TupleToTypeList<column_tuple_decayed>::type;
  Map(Ra child_, F f_) : child(std::move(child_)), f(std::move(f_)) {}
  Ra child;
  F f;
};

template <typename Ra, typename F,
          typename RaDecayed = typename std::decay<Ra>::type,
          typename FDecayed = typename std::decay<F>::type>
Map<RaDecayed, FDecayed> make_map(Ra&& child, F&& f) {
  return Map<RaDecayed, FDecayed>(std::forward<Ra>(child), std::forward<F>(f));
}

template <typename F>
struct MapPipe {
  F f;
};

template <typename F>
MapPipe<typename std::decay<F>::type> map(F&& f) {
  return {std::forward<F>(f)};
}

template <typename Ra, typename F,
          typename RaDecayed = typename std::decay<Ra>::type>
Map<RaDecayed, F> operator|(Ra&& child, MapPipe<F> f) {
  return make_map(std::forward<Ra>(child), std::move(f.f));
}

}  // namespace logical
}  // namespace ra
}  // namespace fluent

#endif  // RA_LOGICAL_MAP_H_
