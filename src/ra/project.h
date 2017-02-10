#ifndef RA_PROJECT_H_
#define RA_PROJECT_H_

#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

namespace fluent {
namespace ra {

template <typename PhysicalChild, std::size_t... Is>
class PhysicalProject {
 public:
  PhysicalProject(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() const {
    return child_.ToRange() | ranges::view::transform([](const auto& t) {
             return std::tuple_cat(std::make_tuple(std::get<Is>(t))...);
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
  explicit Project(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const {
    return make_physical_project<Is...>(child_.ToPhysical());
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
