#ifndef RA_LOGICAL_PROJECT_H_
#define RA_LOGICAL_PROJECT_H_

#include <cstddef>

#include <type_traits>

#include "common/static_assert.h"
#include "common/type_list.h"
#include "common/type_traits.h"
#include "ra/logical/logical_ra.h"

namespace fluent {
namespace ra {
namespace logical {

template <typename Ra, std::size_t... Is>
struct Project : public LogicalRa {
  static_assert(StaticAssert<std::is_base_of<LogicalRa, Ra>>::value, "");
  using child_column_types = typename Ra::column_types;
  using child_len_t = typename TypeListLen<child_column_types>::type;
  static constexpr std::size_t child_len = child_len_t::value;
  static_assert(StaticAssert<All<InRange<Is, 0, child_len>...>>::value, "");

  using column_types =
      typename TypeListProject<child_column_types, Is...>::type;
  explicit Project(Ra child_) : child(std::move(child_)) {}
  Ra child;
};

template <std::size_t... Is, typename Ra,
          typename RaDecayed = typename std::decay<Ra>::type>
Project<RaDecayed, Is...> make_project(Ra&& child) {
  return Project<RaDecayed, Is...>(std::forward<Ra>(child));
}

template <std::size_t... Is>
struct ProjectPipe {};

template <size_t... Is>
ProjectPipe<Is...> project() {
  return {};
}

template <std::size_t... Is, typename Ra,
          typename RaDecayed = typename std::decay<Ra>::type>
Project<RaDecayed, Is...> operator|(Ra&& child, ProjectPipe<Is...>) {
  return make_project<Is...>(std::forward<Ra&&>(child));
}

}  // namespace logical
}  // namespace ra
}  // namespace fluent

#endif  // RA_LOGICAL_PROJECT_H_
