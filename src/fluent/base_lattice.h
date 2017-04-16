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

  // explicit Lattice<T>() : name_("") {
  //   element_ = bot();
  // }
  // explicit Lattice<T>(const std::string &name) : name_(name) {
  //   element_ = bot();
  // }
  // Lattice<T>(const T &e) : name_(""), element_(e) {}
  // explicit Lattice<T>(const std::string &name, const T &e) : name_(name), element_(e) {}
  // Lattice<T>(const Lattice<T> &other) : name_(other.Name()), element_(other.Reveal()) {}
  // Lattice<T>& operator=(const Lattice<T> &other) {
  //   element_ = other.Reveal();
  //   return *this;
  // }

  virtual const std::string& Name() const = 0;

  virtual const T& Reveal() const = 0;

  virtual void merge(const L& other) = 0;

  virtual void merge(const T& other) = 0;

  // template <typename RA>
  // typename std::enable_if<!(std::is_base_of<Lattice<T>, RA>::value)>::type
  // Merge(const RA& ra) {
  //   auto buf = ra::MergeRaInto<RA>(ra);
  //   auto begin = std::make_move_iterator(std::begin(buf));
  //   auto end = std::make_move_iterator(std::end(buf));
  //   for (auto it = begin; it != end; it++) {
  //     merge(std::get<0>(*it));
  //   }
  // }

  // template <typename L>
  // typename std::enable_if<(std::is_base_of<Lattice<T>, L>::value)>::type
  // Merge(const L& l) {
  //   merge(l.Reveal());
  // }

  // void merge(const T &e) {
  //   return do_merge(e);
  // }

  // void merge(const Lattice<T> &l) {
  //   return do_merge(l.Reveal());
  // }

  // const T &bot() const {
  //   return zero;
  // }

  // need this because for now lattice is considered as a collection
  void Tick() {}

  // bool operator==(const Lattice<T>& other) const {
  //   return element_ == other.Reveal();
  // }

  // bool operator<(const Lattice<T>& other) const {
  //   return element_ < other.Reveal();
  // }

  // using type = T;

 // protected:
 //  const std::string name_;
 //  T element_;
 //  const T zero {static_cast<T> (0)};
 //  virtual void do_merge(const T &e) = 0;
};

// Returns whether `l == r` according to the partial order of the lattice.
template <typename T>
typename std::enable_if<
    std::is_base_of<Lattice<T, typename T::lattice_type>, T>::value, bool>::type
operator==(const T& l, const T& r) {
  return l.get() == r.get();
}

// Returns whether `l != r` according to the partial order of the lattice.
template <typename T>
typename std::enable_if<
    std::is_base_of<Lattice<T, typename T::lattice_type>, T>::value, bool>::type
operator!=(const T& l, const T& r) {
  return l.get() != r.get();
}

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
