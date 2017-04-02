#ifndef RA_LINEAGE_TUPLE_H_
#define RA_LINEAGE_TUPLE_H_

#include <cstddef>

#include <ostream>
#include <string>

namespace fluent {
namespace ra {

// The LineageTuple ("a_table", 42) represents a tuple `t` with hash 42 in
// table "a_table". A LineageTuple is essentially just a std::pair<std::string,
// std::size_t>, but we make it a struct to make the code a bit easier to read.
struct LineageTuple {
  std::string collection;
  std::size_t hash;
};

inline std::ostream& operator<<(std::ostream& out, const LineageTuple& lt) {
  out << "(" << lt.collection << ", " << lt.hash << ")";
  return out;
}

inline bool operator==(const LineageTuple& lhs, const LineageTuple& rhs) {
  return lhs.collection == rhs.collection && lhs.hash == rhs.hash;
}

inline bool operator<(const LineageTuple& lhs, const LineageTuple& rhs) {
  if (lhs.collection < rhs.collection) {
    return true;
  } else {
    return lhs.hash < rhs.hash;
  }
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_LINEAGE_TUPLE_H_
