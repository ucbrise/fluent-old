#ifndef RA_PHYSICAL_ITERABLE_H_
#define RA_PHYSICAL_ITERABLE_H_

#include "range/v3/all.hpp"

#include "common/macros.h"
#include "common/type_traits.h"
#include "ra/physical/physical_ra.h"

namespace fluent {
namespace ra {
namespace physical {

template <typename Container>
class Iterable : public PhysicalRa {
 public:
  explicit Iterable(const Container* container) : container_(container) {}
  DISALLOW_COPY_AND_ASSIGN(Iterable);
  DEFAULT_MOVE_AND_ASSIGN(Iterable);

  auto ToRange() { return ranges::view::all(*container_); }

 private:
  const Container* container_;
};

template <typename Container>
Iterable<Container> make_iterable(const Container* container) {
  return Iterable<Container>(container);
}

}  // namespace physical
}  // namespace ra
}  // namespace fluent

#endif  // RA_PHYSICAL_ITERABLE_H_
