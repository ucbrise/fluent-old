#ifndef COLLECTIONS_PERIODIC_H_
#define COLLECTIONS_PERIODIC_H_

#include <cstddef>

#include <algorithm>
#include <array>
#include <chrono>
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

// A Periodic is a two-column collection where the first column is a unique id
// of type `id` and the second column is a time point of type `time`. It looks
// something like this:
//
//   +----+------------+
//   | id | time       |
//   +----+------------+
//   | 09 | 1488762221 |
//   +----+------------+
//   | 10 | 1488762226 |
//   +----+------------+
//   | 11 | 1488762236 |
//   +----+------------+
//
// You cannot write into a Periodic. Instead, Periodics are constructed with a
// period `period` (e.g. 1 second). Then, every `period` (e.g. every 1 second),
// a new tuple is inserted into the table with a unique id and the current
// time. After every tick, the Periodic is cleared.
class Periodic : public Collection {
 public:
  using id = std::size_t;
  using clock = std::chrono::system_clock;
  using time = std::chrono::time_point<clock>;
  using period = std::chrono::milliseconds;

  Periodic(std::string name, period period)
      : name_(std::move(name)), period_(std::move(period)), id_(0) {}
  DISALLOW_COPY_AND_ASSIGN(Periodic);
  DEFAULT_MOVE_AND_ASSIGN(Periodic);

  const std::string& Name() const { return name_; }

  const std::array<std::string, 2>& ColumnNames() const {
    static std::array<std::string, 2> column_names{{"id", "time"}};
    return column_names;
  }

  const period& Period() const { return period_; }

  const std::map<std::tuple<id, time>, CollectionTupleIds>& Get() const {
    return ts_;
  }

  id GetAndIncrementId() { return id_++; }

  void Merge(const std::tuple<id, time>& t, std::size_t hash,
             int logical_time_inserted) {
    MergeCollectionTuple(t, hash, logical_time_inserted, &ts_);
  }

  std::map<std::tuple<id, time>, CollectionTupleIds> Tick() {
    std::map<std::tuple<id, time>, CollectionTupleIds> ts;
    std::swap(ts, ts_);
    return ts;
  }

 private:
  const std::string name_;
  const std::chrono::milliseconds period_;
  id id_;
  std::map<std::tuple<id, time>, CollectionTupleIds> ts_;
};

}  // namespace fluent

#endif  // COLLECTIONS_PERIODIC_H_
