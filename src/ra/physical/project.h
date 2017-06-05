#ifndef RA_PHYSICAL_PROJECT_H_
#define RA_PHYSICAL_PROJECT_H_

#include <type_traits>

#include "range/v3/all.hpp"

#include "common/macros.h"
#include "common/static_assert.h"
#include "common/tuple_util.h"
#include "ra/physical/physical_ra.h"

namespace fluent {
namespace ra {
namespace physical {

template <typename Ra, std::size_t... Is>
class Project : public PhysicalRa {
  static_assert(StaticAssert<std::is_base_of<PhysicalRa, Ra>>::value, "");

 public:
  explicit Project(Ra child) : child_(std::move(child)) {}
  DISALLOW_COPY_AND_ASSIGN(Project);
  DEFAULT_MOVE_AND_ASSIGN(Project);

  auto ToRange() {
    return child_.ToRange() | ranges::view::transform([](const auto& t) {
             return TupleProject<Is...>(t);
           });
  }

 private:
  Ra child_;
};

template <std::size_t... Is, typename Ra,
          typename RaDecayed = typename std::decay<Ra>::type>
Project<RaDecayed, Is...> make_project(Ra&& child) {
  return Project<RaDecayed, Is...>(std::forward<Ra>(child));
}

}  // namespace physical
}  // namespace ra
}  // namespace fluent

#endif  // RA_PHYSICAL_PROJECT_H_
