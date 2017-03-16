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

template <typename T>
class Lattice {
 public:
  explicit Lattice<T>() : name_("") {
    element_ = bot();
  }
  explicit Lattice<T>(const std::string &name) : name_(name) {
    element_ = bot();
  }
  Lattice<T>(const T &e) : name_(""), element_(e) {}
  explicit Lattice<T>(const std::string &name, const T &e) : name_(name), element_(e) {}
  Lattice<T>(const Lattice<T> &other) : name_(other.Name()), element_(other.Reveal()) {}

  const std::string& Name() const { return name_; }

  const T& Reveal() const { return element_; }

  template <typename RA>
  typename std::enable_if<!(std::is_base_of<Lattice<T>, RA>::value)>::type
  Merge(const RA& ra) {
    auto buf = ra::MergeRaInto<RA>(ra);
    auto begin = std::make_move_iterator(std::begin(buf));
    auto end = std::make_move_iterator(std::end(buf));
    for (auto it = begin; it != end; it++) {
      merge(std::get<0>(*it));
    }
  }

  template <typename L>
  typename std::enable_if<(std::is_base_of<Lattice<T>, L>::value)>::type
  Merge(const L& l) {
    merge(l.Reveal());
  }

  void merge(const T &e) {
    return do_merge(e);
  }

  void merge(const Lattice<T> &l) {
    return do_merge(l.Reveal());
  }

  const T &bot() const {
    return zero;
  }

  void Tick() {}

  using type = T;

 protected:
  const std::string name_;
  T element_;
  const T zero {static_cast<T> (0)};
  virtual void do_merge(const T &e) = 0;
};

}  // namespace fluent

#endif  // FLUENT_BASE_LATTICE_H_
