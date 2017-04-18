#ifndef RA_UNBATCH_H_
#define RA_UNBATCH_H_

#include <cstddef>

#include <set>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/type_list.h"

namespace fluent {
namespace ra {

template <typename PhysicalChild, typename LogicalUnbatch>
class PhysicalUnbatch {
 public:
  explicit PhysicalUnbatch(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    return child_.ToRange() | ranges::view::for_each([](const auto& t) {
      return ranges::yield_from(ranges::view::all(std::get<0>(t)));
    }); 
  }

 private:
  PhysicalChild child_;
};

template <typename LogicalChild>
class Unbatch {
 public:
  using column_types = typename TupleToTypeList<typename LogicalChild::column_types::template type<0>::value_type>::type;

  Unbatch(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const {
    return PhysicalUnbatch<
        decltype(child_.ToPhysical()),
        Unbatch<LogicalChild>>(
        child_.ToPhysical());
  }

 private:
  const LogicalChild child_;
};

template <typename LogicalChild>
Unbatch<typename std::decay<LogicalChild>::type> make_unbatch(
    LogicalChild&& child) {
  return {std::forward<LogicalChild>(child)};
}

struct UnbatchPipe {};

UnbatchPipe unbatch() { return {}; }

template <typename LogicalChild>
Unbatch<typename std::decay<LogicalChild>::type> operator|(LogicalChild&& child,
                                                         UnbatchPipe) {
  return make_unbatch(std::forward<LogicalChild>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_UNBATCH_H_
