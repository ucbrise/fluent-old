#ifndef FLUENT_BOOL_LATTICE_H_
#define FLUENT_BOOL_LATTICE_H_

#include "fluent/base_lattice.h"

namespace fluent {

class BoolLattice : public Lattice<BoolLattice, bool> {
public:
	BoolLattice() = default;
	BoolLattice(const std::string &name) : name_(name), element_() {}
	BoolLattice(const bool &e) : name_(""), element_(e) {}
	BoolLattice(const std::string &name, const bool &e) : name_(name), element_(e) {}
	BoolLattice(const BoolLattice &l) = default;
	BoolLattice& operator=(const BoolLattice& l) = default;

	const std::string& Name() const override { return name_; }
	const bool& Reveal() const override { return element_; }
  	void merge(const BoolLattice& l) override { element_ = element_ || l.element_; }
  	void merge(const bool& t) override { element_ = element_ || t; }

 	template <typename RA>
	typename std::enable_if<!(std::is_base_of<Lattice<BoolLattice, bool>, RA>::value)>::type
	Merge(const RA& ra) {
		auto buf = ra::MergeRaInto<RA>(ra);
		auto begin = std::make_move_iterator(std::begin(buf));
		auto end = std::make_move_iterator(std::end(buf));
		for (auto it = begin; it != end; it++) {
		  merge(std::get<0>(*it));
		}
	}

	template <typename L>
	typename std::enable_if<(std::is_base_of<Lattice<BoolLattice, bool>, L>::value)>::type
	Merge(const L& l) {
		merge(l);
	}

	friend bool operator<(const BoolLattice& lhs, const BoolLattice& rhs) {
		return lhs.convert() < rhs.convert();
	}
	// friend bool operator<=(const BoolLattice& lhs, const BoolLattice& rhs) {
	// 	return lhs.convert() <= rhs.convert();
	// }
	friend bool operator>(const BoolLattice& lhs, const BoolLattice& rhs) {
		return lhs.convert() > rhs.convert();
	}
	friend bool operator>=(const BoolLattice& lhs, const BoolLattice& rhs) {
		return lhs.convert() >= rhs.convert();
	}
	friend bool operator==(const BoolLattice& lhs, const BoolLattice& rhs) {
		return lhs.convert() == rhs.convert();
	}
	friend bool operator!=(const BoolLattice& lhs, const BoolLattice& rhs) {
		return lhs.convert() != rhs.convert();
	}

private:
	std::string name_;
	bool element_;
	int convert() const { return element_ ? 1 : 0; }
};

}  // namespace fluent

#endif  // FLUENT_BOOL_LATTICE_H_