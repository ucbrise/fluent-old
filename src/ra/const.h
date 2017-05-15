#ifndef RA_CONST_H_
#define RA_CONST_H_

#include <string>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/hash_util.h"
#include "common/type_list.h"
#include "common/type_traits.h"
#include "ra/lineaged_tuple.h"

// The const operator behaves almost identically to the iterable operator. It
// takes in a pointer to some iterable collection and iterates over it:
//
//   std::vector<std::tuple<int, int>> xs = {{1, 2}, {3, 4}, {5, 6}};
//   auto logical_const = ra::make_const("xs", &xs);
//   auto physical_const = logical_const.ToPhysical();
//   ranges::for_each(physical_const.ToRange(), [](const auto& lt) {
//     std::cout << lt.tuple << std::endl;
//     // (1, 2)
//     // (3, 4)
//     // (5, 6)
//   });
//
// The only difference between const and iterable is that tuples returned by
// const don't have any lineage:
//
//   ranges::for_each(physical_const.ToRange(), [](const auto& lt) {
//     std::cout << lt.lineage << std::endl;
//     // {}
//     // {}
//     // {}
//   });
//
// Why? Well, the const operator represents some constant set of tuples that do
// not belong to any given relation. The lineage of these constant tuples is
// empty. See [1] page 24 for more details.
//
// [1]: https://scholar.google.com/scholar?cluster=14688264622623487965

namespace fluent {
namespace ra {

template <typename T>
class PhysicalConst {
 public:
  explicit PhysicalConst(const T* iterable) : iterable_(iterable) {}

  auto ToRange() {
    return ranges::view::all(*iterable_) |
           ranges::view::transform(
               [this](const auto& t) { return make_lineaged_tuple({}, t); });
  }

 private:
  const T* iterable_;
};

template <typename T>
PhysicalConst<T> make_physical_const(const T* iterable) {
  return PhysicalConst<T>(iterable);
}

template <typename T>
class Const {
 public:
  static_assert(IsTuple<typename T::value_type>::value,
                "Consts must contain tuples.");
  using column_types = typename TupleToTypeList<typename T::value_type>::type;

  Const(std::string name, const T* iterable)
      : name_(std::move(name)), iterable_(iterable) {}

  auto ToPhysical() const { return make_physical_const(iterable_); }

  std::string ToDebugString() const { return name_; }

 private:
  const std::string name_;
  const T* iterable_;
};

template <typename T>
Const<T> make_const(std::string name, const T* iterable) {
  return {std::move(name), iterable};
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_CONST_H_
