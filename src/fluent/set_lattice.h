#ifndef FLUENT_SET_LATTICE_H_
#define FLUENT_SET_LATTICE_H_

#include <algorithm>
#include <iterator>
#include <set>
#include <type_traits>
#include <utility>

#include "ra/iterable.h"
#include "fluent/base_lattice.h"
#include "fluent/max_lattice.h"

namespace fluent {

template <typename... Ts>
class SetLattice : public Lattice<SetLattice<Ts...>, std::set<std::tuple<Ts...>>> {
public:
	SetLattice() = default;
  	SetLattice(const std::string &name) : name_(name), element_() {}
  	SetLattice(const std::set<std::tuple<Ts...>> &e) : name_(""), element_(e) {}
  	SetLattice(const std::string &name, const std::set<std::tuple<Ts...>> &e) : name_(name), element_(e) {}
  	SetLattice(const SetLattice<Ts...> &l) = default;
  	SetLattice& operator=(const SetLattice<Ts...> &l) = default;

	const std::string& Name() const override { return name_; }
	const std::set<std::tuple<Ts...>>& Reveal() const override { return element_; }

  	void merge(const SetLattice<Ts...>& l) override {
	  	for (const auto& t : l.element_) {
	      element_.insert(t);
	    }
  	}
  	void merge(const std::set<std::tuple<Ts...>>& s) override {
  		for (const auto& t : s) {
	      element_.insert(t);
	    }
  	}
  	void merge(const std::tuple<Ts...> t) {
  		element_.insert(t);
  	}

	auto Iterable() const {
		return ra::make_iterable(&element_);
	}

	template <typename RA>
	typename std::enable_if<!(std::is_base_of<Lattice<SetLattice<Ts...>, std::set<std::tuple<Ts...>>>, RA>::value)>::type
	Merge(const RA& ra) {
		auto buf = ra::MergeRaInto<RA>(ra);
		auto begin = std::make_move_iterator(std::begin(buf));
		auto end = std::make_move_iterator(std::end(buf));
		for (auto it = begin; it != end; it++) {
		  merge(std::get<0>(*it));
		}
	}

	template <typename L>
	typename std::enable_if<(std::is_base_of<Lattice<SetLattice<Ts...>, std::set<std::tuple<Ts...>>>, L>::value)>::type
	Merge(const L& l) {
		merge(l);
	}

  	MaxLattice<int> size() const{
  		return MaxLattice<int>(element_.size());
  	}

  	bool operator==(const SetLattice<Ts...>& l) const {
    	return element_ == l.Reveal();
  	}

private:
	std::string name_;
	std::set<std::tuple<Ts...>> element_;

};

}  // namespace fluent

#endif  // FLUENT_SET_LATTICE_H_