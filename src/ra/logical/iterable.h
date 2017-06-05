#ifndef RA_LOGICAL_ITERABLE_H_
#define RA_LOGICAL_ITERABLE_H_

#include <type_traits>

#include "common/static_assert.h"
#include "common/type_list.h"
#include "common/type_traits.h"
#include "ra/logical/logical_ra.h"

namespace fluent {
namespace ra {
namespace logical {

template <typename Container>
struct Iterable : public LogicalRa {
  using is_vector = IsVector<Container>;
  using is_set = IsSet<Container>;
  static_assert(StaticAssert<Or<is_vector, is_set>>::value, "");
  using value_type = typename Container::value_type;
  static_assert(StaticAssert<IsTuple<value_type>>::value, "");

  using column_types = typename TupleToTypeList<value_type>::type;
  explicit Iterable(Container* container_) : container(container_) {}
  Container* container;
};

template <typename Container>
Iterable<Container> make_iterable(Container* container) {
  return Iterable<Container>(container);
}

}  // namespace logical
}  // namespace ra
}  // namespace fluent

#endif  // RA_LOGICAL_ITERABLE_H_
