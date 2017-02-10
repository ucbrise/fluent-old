#ifndef RA_ITERABLE_H_
#define RA_ITERABLE_H_

#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

namespace fluent {
namespace ra {

template <typename T>
class PhysicalIterable {
 public:
  explicit PhysicalIterable(const T* iterable) : iterable_(iterable) {}

  auto ToRange() const { return ranges::view::all(*iterable_); }

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
