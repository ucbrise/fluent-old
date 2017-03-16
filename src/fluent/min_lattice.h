#ifndef FLUENT_MIN_LATTICE_H_
#define FLUENT_MIN_LATTICE_H_

#include "fluent/base_lattice.h"
#include "fluent/bool_lattice.h"

namespace fluent {

template <typename T>
class MinLattice : public Lattice<T> {
protected:
	void do_merge(const T &e) {
		// we need 'this' because this is a templated class
		// 'this' makes the name dependent so that we can access the base definition
		T current = this->element_;
		if (current > e) {
			this->element_ = e;
		}
	}
public:
	explicit MinLattice() : Lattice<T>() { this->element_ = static_cast<T> (1000000); }
	explicit MinLattice(const std::string &name) : Lattice<T>(name) { this->element_ = static_cast<T> (1000000); }
	MinLattice(const T &e) : Lattice<T>(e) {}
	explicit MinLattice(const std::string &name, const T &e) : Lattice<T>(name, e) {}
	MinLattice(const MinLattice &other) : Lattice<T>(other) {}

	BoolLattice lt(T n) const{
		if (this->element_ < n) return BoolLattice(true);
		else return BoolLattice(false);
	}
	BoolLattice lt_eq(T n) const{
		if (this->element_ <= n) return BoolLattice(true);
		else return BoolLattice(false);
	}
	MinLattice<T> add(T n) const{
		return MinLattice<T>(this->element_ + n);
	}
	MinLattice<T> subtract(T n) const{
		return MinLattice<T>(this->element_ - n);
	}
};

}  // namespace fluent

#endif  // FLUENT_MIN_LATTICE_H_