#ifndef COLLECTIONS_SCRATCH_H_
#define COLLECTIONS_SCRATCH_H_

#include <algorithm>
#include <array>
#include <map>
#include <string>
#include <tuple>

#include "collections/collection.h"
#include "collections/collection_tuple_ids.h"
#include "collections/util.h"
#include "common/macros.h"
#include "common/type_traits.h"

namespace fluent {
namespace collections {

template <typename... Ts>
class Scratch : public Collection {
 public:
  Scratch(std::string name, std::array<std::string, sizeof...(Ts)> column_names)
      : name_(std::move(name)), column_names_(std::move(column_names)) {}
  DISALLOW_COPY_AND_ASSIGN(Scratch);
  DEFAULT_MOVE_AND_ASSIGN(Scratch);

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

  std::map<std::tuple<Ts...>, CollectionTupleIds> Tick() {
    std::map<std::tuple<Ts...>, CollectionTupleIds> ts;
    std::swap(ts, ts_);
    return ts;
  }

 private:
  const std::string name_;
  const std::array<std::string, sizeof...(Ts)> column_names_;
  std::map<std::tuple<Ts...>, CollectionTupleIds> ts_;
};

}  // namespace collections
}  // namespace fluent

#endif  // COLLECTIONS_SCRATCH_H_
