#ifndef COLLECTIONS_STDIN_H_
#define COLLECTIONS_STDIN_H_

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "zmq.hpp"

#include "collections/collection.h"
#include "collections/collection_tuple_ids.h"
#include "collections/util.h"
#include "common/macros.h"
#include "common/type_traits.h"

namespace fluent {
namespace collections {

class Stdin : public Collection {
 public:
  Stdin() {}
  DISALLOW_COPY_AND_ASSIGN(Stdin);
  DEFAULT_MOVE_AND_ASSIGN(Stdin);

  const std::string& Name() const {
    static const std::string name = "stdin";
    return name;
  }

  const std::array<std::string, 1>& ColumnNames() const {
    static std::array<std::string, 1> column_names{{"stdin"}};
    return column_names;
  }

  const std::map<std::tuple<std::string>, CollectionTupleIds>& Get() const {
    return lines_;
  }

  zmq::pollitem_t Pollitem() {
    return {/* socket */ nullptr, /* fd */ 0, /* events */ ZMQ_POLLIN,
            /* revents */ 0};
  }

  static std::tuple<std::string> ReadLine() {
    std::string line;
    std::getline(std::cin, line);
    return std::tuple<std::string>(line);
  }

  void Merge(const std::tuple<std::string>& t, std::size_t hash,
             int logical_time_inserted) {
    MergeCollectionTuple(t, hash, logical_time_inserted, &lines_);
  }

  std::map<std::tuple<std::string>, CollectionTupleIds> Tick() {
    std::map<std::tuple<std::string>, CollectionTupleIds> lines;
    std::swap(lines_, lines);
    return lines;
  }

 private:
  std::map<std::tuple<std::string>, CollectionTupleIds> lines_;
};

}  // namespace collections
}  // namespace fluent

#endif  // COLLECTIONS_STDIN_H_
