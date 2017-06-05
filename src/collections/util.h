#ifndef COLLECTIONS_UTIL_H_
#define COLLECTIONS_UTIL_H_

#include <cstddef>

#include <map>
#include <tuple>

#include "glog/logging.h"

#include "collections/collection_tuple_ids.h"

namespace fluent {

template <typename... Ts>
void MergeCollectionTuple(const std::tuple<Ts...>& t, const std::size_t hash,
                          const int logical_time_inserted,
                          std::map<std::tuple<Ts...>, CollectionTupleIds>* ts) {
  auto iter = ts->find(t);
  if (iter == ts->end()) {
    CollectionTupleIds ids = CollectionTupleIds{hash, {logical_time_inserted}};
    ts->insert(std::make_pair(t, ids));
  } else {
    CHECK_EQ(iter->second.hash, hash);
    iter->second.logical_times_inserted.insert(logical_time_inserted);
  }
}

}  // namespace fluent

#endif  // COLLECTIONS_UTIL_H_
