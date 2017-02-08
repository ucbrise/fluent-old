#ifndef RA_ITERABLE_H_
#define RA_ITERABLE_H_

#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

namespace fluent {
namespace ra {

template <typename Rng>
class PhysicalIterable {
 public:
  explicit PhysicalIterable(Rng r) : r_(std::move(r)) {}

  auto ToRange() const { return r_; }

 private:
  Rng r_;
};

template <typename Rng>
PhysicalIterable<typename std::decay<Rng>::type> make_physical_iterable(
    Rng&& r) {
  return PhysicalIterable<typename std::decay<Rng>::type>(std::forward<Rng>(r));
}

template <typename T>
class Iterable {
 public:
  explicit Iterable(T* iterable) : iterable_(iterable) {}

  auto ToPhysical() const {
    return make_physical_iterable(ranges::view::all(*iterable_));
  }

 private:
  T* iterable_;
};

template <typename T>
Iterable<T> make_iterable(T* iterable) {
  return Iterable<T>(iterable);
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_ITERABLE_H_
