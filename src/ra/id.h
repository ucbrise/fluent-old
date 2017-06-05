#ifndef RA_ID_H_
#define RA_ID_H_

#include <type_traits>
#include <utility>

#include "fmt/format.h"
#include "range/v3/all.hpp"

#include "common/hash_util.h"
#include "common/type_list.h"
#include "ra/lineaged_tuple.h"

namespace fluent {
namespace ra {

template <typename PhysicalChild>
class PhysicalId {
 public:
  explicit PhysicalId(PhysicalChild child) : child_(child) {}

  auto ToRange() { return child_.ToRange(); }

 private:
  PhysicalChild child_;
};

template <typename PhysicalChild>
PhysicalId<typename std::decay<PhysicalChild>::type> make_physical_id(
    PhysicalChild&& child) {
  using decayed = typename std::decay<PhysicalChild>::type;
  return PhysicalId<decayed>(std::forward<PhysicalChild>(child));
}

template <typename LogicalChild>
class Id {
 public:
  using column_types = typename LogicalChild::column_types;

  explicit Id(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const { return make_physical_id(child_.ToPhysical()); }

  std::string ToDebugString() const {
    return fmt::format("Id({})", child_.ToDebugString());
  }

 private:
  const LogicalChild child_;
};

template <typename LogicalChild>
Id<typename std::decay<LogicalChild>::type> make_id(LogicalChild&& child) {
  using decayed = typename std::decay<LogicalChild>::type;
  return Id<decayed>(std::forward<LogicalChild>(child));
}

struct IdPipe {};

IdPipe id() { return {}; }

template <typename LogicalChild>
Id<typename std::decay<LogicalChild>::type> operator|(LogicalChild&& child,
                                                      IdPipe) {
  return make_id(std::forward<LogicalChild>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_ID_H_
