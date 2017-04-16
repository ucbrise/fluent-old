#ifndef FLUENT_MAP_LATTICE_H_
#define FLUENT_MAP_LATTICE_H_

#include "ra/map_iterable.h"
#include "fluent/base_lattice.h"
#include "fluent/max_lattice.h"

#include <unordered_map>

namespace fluent {

template <typename K, typename V>
class MapLattice : public Lattice<MapLattice<K, V>, std::unordered_map<K, V>> {
public:
	MapLattice() = default;
  	MapLattice(const std::string &name) : name_(name), element_() {}
  	MapLattice(const std::unordered_map<K, V> &e) : name_(""), element_(e) {}
  	MapLattice(const std::string &name, const std::unordered_map<K, V> &e) : name_(name), element_(e) {}
  	MapLattice(const MapLattice<K, V> &l) = default;
  	MapLattice& operator=(const MapLattice<K, V> &l) = default;

	const std::string& Name() const override { return name_; }
	const std::unordered_map<K, V>& Reveal() const override { return element_; }

  	void merge(const MapLattice<K, V>& l) override {
	  	for (const auto& kv : l.element_) {
	      const K& key = std::get<0>(kv);
	      const V& value = std::get<1>(kv);
	      insert_pair(key, value);
	    }
  	}
  	void merge(const std::unordered_map<K, V>& m) override {
  		for (const auto& kv : m) {
	      const K& key = std::get<0>(kv);
	      const V& value = std::get<1>(kv);
	      insert_pair(key, value);
	    }
  	}
  	void merge(const K &k, const V &v) {
  		insert_pair(k, v);
  	}

  	void insert_pair(const K &k, const V &v) {
  		if (element_.count(k) == 0) {
	        element_.insert(std::make_pair(k, v));
	        //ts_.insert(std::make_tuple(k, v.Reveal()));
	    } else {
	    	//ts_.erase(std::make_tuple(k, element_.at(k).Reveal()));
	        element_[k].merge(v);
	        //ts_.insert(std::make_tuple(k, element_.at(k).Reveal()));
	    }
  	}

	// ra::Iterable<std::set<std::tuple<K, typename V::lattice_type>>> Iterable() const {
	// 	return ra::make_iterable(&(this->ts_));
	// }
  	//template <typename T>
	auto Iterable() const {
		return ra::make_map_iterable(&element_);
	}

	template <typename RA>
	typename std::enable_if<!(std::is_base_of<Lattice<MapLattice<K, V>, std::unordered_map<K, V>>, RA>::value)>::type
	Merge(const RA& ra) {
		auto buf = ra::MergeRaInto<RA>(ra);
		auto begin = std::make_move_iterator(std::begin(buf));
		auto end = std::make_move_iterator(std::end(buf));
		for (auto it = begin; it != end; it++) {
		  merge(std::get<0>(*it), std::get<1>(*it));
		}
	}

	template <typename L>
	typename std::enable_if<(std::is_base_of<Lattice<MapLattice<K, V>, std::unordered_map<K, V>>, L>::value)>::type
	Merge(const L& l) {
		merge(l);
	}

  	MaxLattice<int> size() const{
  		return MaxLattice<int>(element_.size());
  	}

	V &at(K k) {
		return element_[k];
	}

  	bool operator==(const MapLattice<K, V>& l) const {
    	return element_ == l.Reveal();
  	}

private:
	std::string name_;
	std::unordered_map<K, V> element_;
	//TODO: think about how to make this more efficient and generalizable
	//std::set<std::tuple<K, typename V::lattice_type>> ts_;

};

}  // namespace fluent

#endif  // FLUENT_MAP_LATTICE_H_