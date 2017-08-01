#ifndef COLLECTIONS_STDOUT_H_
#define COLLECTIONS_STDOUT_H_

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
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

class Stdout : public Collection {
 public:
  Stdout() {}
  DISALLOW_COPY_AND_ASSIGN(Stdout);
  DEFAULT_MOVE_AND_ASSIGN(Stdout);

  const std::string& Name() const {
    static const std::string name = "stdout";
    return name;
  }

  const std::array<std::string, 1>& ColumnNames() const {
    static std::array<std::string, 1> column_names{{"stdout"}};
    return column_names;
  }

  void Merge(const std::tuple<std::string>& t, std::size_t hash,
             int logical_time_inserted) {
    UNUSED(hash);
    UNUSED(logical_time_inserted);
    std::cout << std::get<0>(t) << std::endl;
  }

  void DeferredMerge(const std::tuple<std::string>& t, std::size_t hash,
                     int logical_time_inserted) {
    UNUSED(hash);
    UNUSED(logical_time_inserted);
    deferred_merge_.insert(t);
  }

  std::map<std::tuple<std::string>, CollectionTupleIds> Tick() {
    for (const std::tuple<std::string>& t : deferred_merge_) {
      std::cout << std::get<0>(t) << std::endl;
    }
    deferred_merge_.clear();
    return {};
  }

 private:
  std::set<std::tuple<std::string>> deferred_merge_;
};

}  // namespace collections
}  // namespace fluent

#endif  // COLLECTIONS_STDOUT_H_
