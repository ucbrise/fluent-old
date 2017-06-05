#ifndef RA_MAP_H_
#define RA_MAP_H_

#include <type_traits>
#include <utility>

#include "fmt/format.h"
#include "range/v3/all.hpp"

#include "common/hash_util.h"
#include "common/type_list.h"
#include "ra/lineaged_tuple.h"

namespace fluent {
namespace ra {

template <typename PhysicalChild, typename F>
class PhysicalMap {
 public:
  PhysicalMap(PhysicalChild child, F* f) : child_(child), f_(f) {}

  auto ToRange() {
    auto r = child_.ToRange() | ranges::view::transform([this](auto lt) {
               return make_lineaged_tuple(std::move(lt.lineage),
                                          (*f_)(std::move(lt.tuple)));
             });
    return r;
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
  using child_column_types = typename LogicalChild::column_types;
  using child_tuple_type = typename TypeListToTuple<child_column_types>::type;
  using tuple_type = typename std::result_of<F(child_tuple_type)>::type;
  using column_types = typename TupleToTypeList<tuple_type>::type;

  Map(LogicalChild child, F f) : child_(std::move(child)), f_(std::move(f)) {}

  auto ToPhysical() const {
    return make_physical_map(child_.ToPhysical(), &f_);
  }

  std::string ToDebugString() const {
    return fmt::format("Map({})", child_.ToDebugString());
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
