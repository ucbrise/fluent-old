#ifndef RA_LOGICAL_META_COLLECTION_H_
#define RA_LOGICAL_META_COLLECTION_H_

#include <type_traits>

#include "collections/collection.h"
#include "collections/collection_util.h"
#include "common/static_assert.h"
#include "fluent/local_tuple_id.h"
#include "ra/logical/logical_ra.h"

namespace fluent {
namespace ra {
namespace logical {

template <typename C>
struct MetaCollection : public LogicalRa {
  using is_base_of = std::is_base_of<fluent::Collection, C>;
  static_assert(StaticAssert<is_base_of>::value, "");
  using collection_types = typename CollectionTypes<C>::type;
  using collection_tuple = typename TypeListToTuple<collection_types>::type;

  using column_types = TypeList<collection_tuple, LocalTupleId>;
  explicit MetaCollection(const C* collection_) : collection(collection_) {}
  const C* collection;
};

template <typename C>
MetaCollection<C> make_meta_collection(const C* collection) {
  return MetaCollection<C>(collection);
}

}  // namespace logical
}  // namespace ra
}  // namespace fluent

#endif  // RA_LOGICAL_META_COLLECTION_H_
