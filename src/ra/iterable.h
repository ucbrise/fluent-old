#ifndef RA_ITERABLE_H_
#define RA_ITERABLE_H_

#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/type_list.h"

namespace fluent {
namespace ra {

template <typename T>
class PhysicalIterable {
 public:
  explicit PhysicalIterable(const T* iterable) : iterable_(iterable) {}

  auto ToRange() { return ranges::view::all(*iterable_); }

 private:
  const T* iterable_;
};

template <typename T>
PhysicalIterable<T> make_physical_iterable(const T* iterable) {
  return PhysicalIterable<T>(iterable);
}

template <typename T>
class Iterable {
 public:
  using column_types = typename TupleToTypeList<typename T::value_type>::type;

  explicit Iterable(const T* iterable) : iterable_(iterable) {}

  auto ToPhysical() const { return make_physical_iterable(iterable_); }

 private:
  const T* iterable_;
};

template <typename T>
Iterable<T> make_iterable(const T* iterable) {
  return Iterable<T>(iterable);
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_ITERABLE_H_
