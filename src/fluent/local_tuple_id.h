#ifndef FLUENT_TUPLE_ID_H_
#define FLUENT_TUPLE_ID_H_

#include <cstddef>

#include <iostream>
#include <string>

#include "common/tuple_util.h"

namespace fluent {

struct LocalTupleId {
  std::string collection_name;
  std::size_t hash;
  int logical_time_inserted;
};

namespace {

std::tuple<const std::string&, std::size_t, int> ToTuple(
    const LocalTupleId& id) {
  return std::tuple<const std::string&, std::size_t, int>(
      id.collection_name, id.hash, id.logical_time_inserted);
}

}  // namespace

inline bool operator==(const LocalTupleId& lhs, const LocalTupleId& rhs) {
  return ToTuple(lhs) == ToTuple(rhs);
}

inline bool operator!=(const LocalTupleId& lhs, const LocalTupleId& rhs) {
  return ToTuple(lhs) != ToTuple(rhs);
}

inline bool operator<(const LocalTupleId& lhs, const LocalTupleId& rhs) {
  return ToTuple(lhs) < ToTuple(rhs);
}

inline bool operator<=(const LocalTupleId& lhs, const LocalTupleId& rhs) {
  return ToTuple(lhs) <= ToTuple(rhs);
}

inline bool operator>(const LocalTupleId& lhs, const LocalTupleId& rhs) {
  return ToTuple(lhs) > ToTuple(rhs);
}

inline bool operator>=(const LocalTupleId& lhs, const LocalTupleId& rhs) {
  return ToTuple(lhs) >= ToTuple(rhs);
}

inline std::ostream& operator<<(std::ostream& out, const LocalTupleId& id) {
  using fluent::common::operator<<;
  out << ToTuple(id);
  return out;
}

}  // namespace fluent

#endif  // FLUENT_TUPLE_ID_H_
