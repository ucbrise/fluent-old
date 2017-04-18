#ifndef FLUENT_BASE_LATTICE_H_
#define FLUENT_BASE_LATTICE_H_

#include <algorithm>
#include <iterator>
#include <set>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "ra/iterable.h"
#include "ra/ra_util.h"

namespace fluent {

template <typename L, typename T>
class Lattice {
 public:
  using lattice_type = T;

  virtual const std::string& Name() const = 0;

  virtual const T& Reveal() const = 0;

  virtual void merge(const L& other) = 0;

  virtual void merge(const T& other) = 0;

  // need this because for now lattice is considered as a collection
  void Tick() {}

};

// Returns whether `l == r` according to the partial order of the lattice.
template <typename T>
typename std::enable_if<
    std::is_base_of<Lattice<T, typename T::lattice_type>, T>::value, bool>::type
operator==(const T& l, const T& r) {
  return l.Reveal() == r.Reveal();
}

// Returns whether `l != r` according to the partial order of the lattice.
template <typename T>
typename std::enable_if<
    std::is_base_of<Lattice<T, typename T::lattice_type>, T>::value, bool>::type
operator!=(const T& l, const T& r) {
  return l.Reveal() != r.Reveal();
}

// Commented out due to overloaded "<=" operator
// Returns true if `l <= r` according to the partial order of the lattice.
// template <typename T>
// typename std::enable_if<
//     std::is_base_of<Lattice<T, typename T::lattice_type>, T>::value, bool>::type
// operator<=(T l, const T& r) {
//   l.join(r);
//   return l.get() == r.get();
// }

}  // namespace fluent

#endif  // FLUENT_BASE_LATTICE_H_
