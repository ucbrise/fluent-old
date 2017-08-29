#ifndef COLLECTIONS_TABLE_H_
#define COLLECTIONS_TABLE_H_

#include <algorithm>
#include <array>
#include <iterator>
#include <map>
#include <set>
#include <type_traits>
#include <utility>

#include "glog/logging.h"

#include "collections/collection.h"
#include "collections/collection_tuple_ids.h"
#include "collections/util.h"
#include "common/macros.h"
#include "common/type_traits.h"

namespace fluent {
namespace collections {

template <typename... Ts>
class Table : public Collection {
 public:
  Table(std::string name, std::array<std::string, sizeof...(Ts)> column_names)
      : name_(std::move(name)), column_names_(std::move(column_names)) {}
  DISALLOW_COPY_AND_ASSIGN(Table);
  DEFAULT_MOVE_AND_ASSIGN(Table);

  const std::string& Name() const { return name_; }

  const std::array<std::string, sizeof...(Ts)>& ColumnNames() const {
    return column_names_;
  }

  const std::map<std::tuple<Ts...>, CollectionTupleIds>& Get() const {
    return ts_;
  }

  void Merge(const std::tuple<Ts...>& t, std::size_t hash,
             int logical_time_inserted) {
    MergeCollectionTuple(t, hash, logical_time_inserted, &ts_);
  }

  void DeferredMerge(const std::tuple<Ts...>& t, std::size_t hash,
                     int logical_time_inserted) {
    MergeCollectionTuple(t, hash, logical_time_inserted, &deferred_merge_);
  }

  void DeferredDelete(const std::tuple<Ts...>& t, std::size_t hash,
                      int logical_time_inserted) {
    MergeCollectionTuple(t, hash, logical_time_inserted, &deferred_delete_);
  }

  std::map<std::tuple<Ts...>, CollectionTupleIds> Tick() {
    // Merge deferred_merge_ into ts_.
    for (const auto& pair : deferred_merge_) {
      const std::tuple<Ts...>& t = pair.first;
      const CollectionTupleIds& ids = pair.second;
      auto iter = ts_.find(t);
      if (iter == ts_.end()) {
        ts_.insert(pair);
      } else {
        CHECK_EQ(iter->second.hash, ids.hash);
        auto begin = ids.logical_times_inserted.begin();
        auto end = ids.logical_times_inserted.end();
        iter->second.logical_times_inserted.insert(begin, end);
      }
    }

    // Delete deferred_delete_ from ts_.
    std::map<std::tuple<Ts...>, CollectionTupleIds> deleted;
    for (const auto& pair : deferred_delete_) {
      const std::tuple<Ts...>& t = pair.first;
      const CollectionTupleIds& ids = pair.second;
      auto iter = ts_.find(t);
      if (iter == ts_.end()) {
        // Do nothing.
      } else {
        CHECK_EQ(iter->second.hash, ids.hash);
        deleted.insert(*iter);
        ts_.erase(iter);
      }
    }

    deferred_merge_.clear();
    deferred_delete_.clear();
    return deleted;
  }

 private:
  const std::string name_;
  const std::array<std::string, sizeof...(Ts)> column_names_;
  std::map<std::tuple<Ts...>, CollectionTupleIds> ts_;
  std::map<std::tuple<Ts...>, CollectionTupleIds> deferred_merge_;
  std::map<std::tuple<Ts...>, CollectionTupleIds> deferred_delete_;
};

}  // namespace collections
}  // namespace fluent

#endif  // COLLECTIONS_TABLE_H_
