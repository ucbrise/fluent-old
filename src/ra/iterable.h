#ifndef RA_ITERABLE_H_
#define RA_ITERABLE_H_

#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/type_list.h"
#include "common/type_traits.h"

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
  static_assert(IsTuple<typename T::value_type>::value,
                "Iterables must contain tuples.");
  using column_types = typename TupleToTypeList<typename T::value_type>::type;

  Iterable(std::string name, const T* iterable)
      : name_(std::move(name)), iterable_(iterable) {}

  auto ToPhysical() const { return make_physical_iterable(iterable_); }

  std::string ToDebugString() const { return name_; }

 private:
  const std::string name_;
  const T* iterable_;
};

template <typename T>
Iterable<T> make_iterable(std::string name, const T* iterable) {
  return {std::move(name), iterable};
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_ITERABLE_H_
