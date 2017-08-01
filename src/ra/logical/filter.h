#ifndef RA_LOGICAL_FILTER_H_
#define RA_LOGICAL_FILTER_H_

#include <type_traits>

#include "common/static_assert.h"
#include "common/type_list.h"
#include "common/type_traits.h"
#include "ra/logical/logical_ra.h"

namespace fluent {
namespace ra {
namespace logical {

template <typename Ra, typename F>
struct Filter : public LogicalRa {
  static_assert(common::StaticAssert<std::is_base_of<LogicalRa, Ra>>::value,
                "");
  using child_column_types = typename Ra::column_types;
  using child_column_tuple =
      typename common::TypeListToTuple<child_column_types>::type;
  static_assert(
      common::StaticAssert<common::IsInvocable<F, child_column_tuple>>::value,
      "");
  using f_return = typename std::result_of<F(child_column_tuple)>::type;
  static_assert(common::StaticAssert<std::is_same<f_return, bool>>::value, "");

  using column_types = child_column_types;
  Filter(Ra child_, F f_) : child(std::move(child_)), f(std::move(f_)) {}
  Ra child;
  F f;
};

template <typename Ra, typename F,
          typename RaDecayed = typename std::decay<Ra>::type,
          typename FDecayed = typename std::decay<F>::type>
Filter<RaDecayed, FDecayed> make_filter(Ra&& child, F&& f) {
  return Filter<RaDecayed, FDecayed>{std::forward<Ra>(child),
                                     std::forward<F>(f)};
}

template <typename F>
struct FilterPipe {
  F f;
};

template <typename F>
FilterPipe<typename std::decay<F>::type> filter(F&& f) {
  return {std::forward<F>(f)};
}

template <typename Ra, typename F,
          typename RaDecayed = typename std::decay<Ra>::type>
Filter<typename std::decay<Ra>::type, F> operator|(Ra&& child,
                                                   FilterPipe<F> f) {
  return make_filter(std::forward<Ra&&>(child), std::move(f.f));
}

}  // namespace logical
}  // namespace ra
}  // namespace fluent

#endif  // RA_LOGICAL_FILTER_H_
