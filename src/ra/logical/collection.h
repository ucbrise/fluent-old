#ifndef RA_LOGICAL_COLLECTION_H_
#define RA_LOGICAL_COLLECTION_H_

#include <type_traits>

#include "collections/collection.h"
#include "collections/collection_util.h"
#include "common/static_assert.h"
#include "ra/logical/logical_ra.h"

namespace fluent {
namespace ra {
namespace logical {

template <typename C>
struct Collection : public LogicalRa {
  using is_base_of = std::is_base_of<collections::Collection, C>;
  static_assert(common::StaticAssert<is_base_of>::value, "");

  using column_types = typename collections::CollectionTypes<C>::type;
  explicit Collection(const C* collection_) : collection(collection_) {}
  const C* collection;
};

template <typename C>
Collection<C> make_collection(const C* collection) {
  return Collection<C>(collection);
}

}  // namespace logical
}  // namespace ra
}  // namespace fluent

#endif  // RA_LOGICAL_COLLECTION_H_
