#ifndef FLUENT_MAP_LATTICE_H_
#define FLUENT_MAP_LATTICE_H_

#include <algorithm>
#include <iterator>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "fluent/max_lattice.h"
#include "ra/iterable.h"
#include "ra/ra_util.h"

namespace fluent {

template <typename K, typename V>
class MapLattice {
 public:
  explicit MapLattice<K, V>() : name_("") {}
  explicit MapLattice<K, V>(const std::string &name) : name_(name) {}
  MapLattice<K, V>(const std::unordered_map<K, V> &e) : name_(""), element_(e) {}
  explicit MapLattice<K, V>(const std::string &name, const std::unordered_map<K, V> &e) : name_(name), element_(e) {}
  MapLattice<K, V>(const MapLattice<K, V> &other) : name_(other.Name()), element_(other.Reveal()) {}
  MapLattice<K, V>& operator=(const MapLattice<K, V> &other) {
    element_ = other.Reveal();
    return *this;
  }

  const std::string& Name() const { return name_; }

  const std::unordered_map<K, V>& Reveal() const { return element_; }

  ra::Iterable<std::set<std::tuple<K, typename V::type>>> Iterable() const {
    return ra::make_iterable(&(this->ts_));
  }

  template <typename RA>
  void Merge(const RA& ra) {
    auto buf = ra::MergeRaInto<RA>(ra);
    auto begin = std::make_move_iterator(std::begin(buf));
    auto end = std::make_move_iterator(std::end(buf));
    for (auto it = begin; it != end; it++) {
      merge(std::get<0>(*it), std::get<1>(*it));
    }
  }

  // void merge(const K &k, const typename V::type &v) {
  //   return do_merge(k, v);
  // }

  void merge(const K &k, const V &v) {
    return do_merge(k, v);
  }

  void merge(const std::unordered_map<K, V> &e) {
    return do_merge(e);
  }

  void merge(const MapLattice<K, V> &l) {
    return do_merge(l.Reveal());
  }

  // const std::unordered_map<K, V> &bot() const {
  //   return zero;
  // }

  void Tick() {}

  MaxLattice<int> size() const{
		return this->element_.size();
  }

  V &at(K k) {
	return element_[k];
  }

  bool operator==(const MapLattice<K, V>& other) const {
    return element_ == other.Reveal();
  }

 protected:
  const std::string name_;
  std::unordered_map<K, V> element_;
  std::set<std::tuple<K, typename V::type>> ts_;
  //const std::unordered_map<K, V> zero {static_cast<std::unordered_map<K, V>> (0)};
  void insert_pair(const K &k, const V &v) {
      auto search = this->element_.find(k);
      if (search != this->element_.end()) {
      	  this->ts_.erase(std::make_tuple(k, this->element_.at(k).Reveal()));
          // avoid copying the value out of the pair during casting!  Instead
          // cast the pointer. A bit ugly but seems like it should be safe.
          static_cast<V *>(&(search->second))->merge(v);
          this->ts_.insert(std::make_tuple(k, this->element_.at(k).Reveal()));

      } else {
          // need to copy v since we will be "growing" it within the lattice
          //V new_v = v;
          this->element_.emplace(k, v);
          this->ts_.insert(std::make_tuple(k, v.Reveal()));
      }
  }
  void do_merge(const std::unordered_map<K, V> &m) {
      for (auto ms = m.begin(); ms != m.end(); ++ms) {
          this->insert_pair(ms->first, ms->second);
      }
  }
  // void do_merge(const K &k, const typename V::type &v) {
  //     this->insert_pair(k, v);
  // }
  void do_merge(const K &k, const V &v) {
      this->insert_pair(k, v);
  }
};

}  // namespace fluent

#endif  // FLUENT_MAP_LATTICE_H_