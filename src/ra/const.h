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
