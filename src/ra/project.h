#ifndef RA_PROJECT_H_
#define RA_PROJECT_H_

#include <type_traits>
#include <utility>

#include "fmt/format.h"
#include "range/v3/all.hpp"

#include "common/hash_util.h"
#include "common/string_util.h"
#include "common/tuple_util.h"
#include "common/type_list.h"
#include "ra/lineaged_tuple.h"

namespace fluent {
namespace ra {

template <typename PhysicalChild, std::size_t... Is>
class PhysicalProject {
 public:
  PhysicalProject(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    return child_.ToRange() | ranges::view::transform([](auto lt) {
             return make_lineaged_tuple(std::move(lt.lineage),
                                        TupleProject<Is...>(lt.tuple));
           });
  }

 private:
  PhysicalChild child_;
};

template <std::size_t... Is, typename PhysicalChild>
PhysicalProject<typename std::decay<PhysicalChild>::type, Is...>
make_physical_project(PhysicalChild&& child) {
  return PhysicalProject<typename std::decay<PhysicalChild>::type, Is...>(
      std::forward<PhysicalChild>(child));
}

template <typename LogicalChild, std::size_t... Is>
class Project {
 public:
  using child_column_types = typename LogicalChild::column_types;
  using column_types =
      typename TypeListProject<child_column_types, Is...>::type;

  explicit Project(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const {
    return make_physical_project<Is...>(child_.ToPhysical());
  }

  std::string ToDebugString() const {
    return fmt::format("Project<{}>({})", Join(Is...), child_.ToDebugString());
  }

 private:
  const LogicalChild child_;
};

template <std::size_t... Is, typename LogicalChild>
Project<typename std::decay<LogicalChild>::type, Is...> make_project(
    LogicalChild&& child) {
  return Project<typename std::decay<LogicalChild>::type, Is...>(
      std::forward<LogicalChild>(child));
}

template <std::size_t... Is>
struct ProjectPipe {};

template <size_t... Is>
ProjectPipe<Is...> project() {
  return {};
}

template <std::size_t... Is, typename LogicalChild>
Project<typename std::decay<LogicalChild>::type, Is...> operator|(
    LogicalChild&& child, ProjectPipe<Is...>) {
  return make_project<Is...>(std::forward<LogicalChild&&>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_PROJECT_H_
