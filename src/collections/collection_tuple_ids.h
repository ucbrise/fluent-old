#ifndef COLLECTIONS_COLLECTION_TUPLE_IDS_H_
#define COLLECTIONS_COLLECTION_TUPLE_IDS_H_

#include <cstddef>

#include <set>

#include "common/collection_util.h"
#include "common/tuple_util.h"

namespace fluent {
namespace collections {

struct CollectionTupleIds {
  std::size_t hash;
  std::set<int> logical_times_inserted;
};

namespace {

std::tuple<std::size_t, const std::set<int>&> ToTuple(
    const CollectionTupleIds& ids) {
  return std::tuple<std::size_t, const std::set<int>&>(
      ids.hash, ids.logical_times_inserted);
}

}  // namespace

inline bool operator==(const CollectionTupleIds& lhs,
                       const CollectionTupleIds& rhs) {
  return ToTuple(lhs) == ToTuple(rhs);
}

inline bool operator!=(const CollectionTupleIds& lhs,
                       const CollectionTupleIds& rhs) {
  return ToTuple(lhs) != ToTuple(rhs);
}

inline bool operator<(const CollectionTupleIds& lhs,
                      const CollectionTupleIds& rhs) {
  return ToTuple(lhs) < ToTuple(rhs);
}

inline bool operator<=(const CollectionTupleIds& lhs,
                       const CollectionTupleIds& rhs) {
  return ToTuple(lhs) <= ToTuple(rhs);
}

inline bool operator>(const CollectionTupleIds& lhs,
                      const CollectionTupleIds& rhs) {
  return ToTuple(lhs) > ToTuple(rhs);
}

inline bool operator>=(const CollectionTupleIds& lhs,
                       const CollectionTupleIds& rhs) {
  return ToTuple(lhs) >= ToTuple(rhs);
}

inline std::ostream& operator<<(std::ostream& out,
                                const CollectionTupleIds& ids) {
  using fluent::common::operator<<;
  out << ToTuple(ids);
  return out;
}

}  // namespace collections
}  // namespace fluent

#endif  // COLLECTIONS_COLLECTION_TUPLE_IDS_H_
