#ifndef RA_LINEAGED_TUPLE_H_
#define RA_LINEAGED_TUPLE_H_

#include <ostream>
#include <set>
#include <string>
#include <tuple>
#include <utility>

#include "common/collection_util.h"
#include "common/tuple_util.h"
#include "ra/lineage_tuple.h"

namespace fluent {
namespace ra {

template <typename... Ts>
struct LineagedTuple {
  std::set<LineageTuple> lineage;
  std::tuple<Ts...> tuple;
};

template <typename... Ts>
bool operator==(const LineagedTuple<Ts...>& lhs,
                const LineagedTuple<Ts...>& rhs) {
  return lhs.lineage == rhs.lineage && lhs.tuple == rhs.tuple;
}

template <typename... Ts>
bool operator<(const LineagedTuple<Ts...>& lhs,
               const LineagedTuple<Ts...>& rhs) {
  return (lhs.lineage < rhs.lineage) ||
         (lhs.lineage == rhs.lineage && lhs.tuple < rhs.tuple);
}

template <typename... Ts>
LineagedTuple<Ts...> make_lineaged_tuple(std::set<LineageTuple> lineage,
                                         std::tuple<Ts...> tuple) {
  return {std::move(lineage), std::move(tuple)};
}

template <typename... Ts>
std::ostream& operator<<(std::ostream& out, const LineagedTuple<Ts...>& lt) {
  using fluent::operator<<;
  out << "(" << lt.lineage << ", " << lt.tuple << ")";
  return out;
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_LINEAGED_TUPLE_H_
