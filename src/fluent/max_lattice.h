#ifndef FLUENT_MAX_LATTICE_H_
#define FLUENT_MAX_LATTICE_H_

#include "fluent/base_lattice.h"
#include "fluent/bool_lattice.h"

namespace fluent {

template <typename T>
class MaxLattice : public Lattice<T> {
protected:
	void do_merge(const T &e) {
		// we need 'this' because this is a templated class
		// 'this' makes the name dependent so that we can access the base definition
		T current = this->element_;
		if (current < e) {
			this->element_ = e;
		}
	}
public:
	explicit MaxLattice() : Lattice<T>() {}
	explicit MaxLattice(const std::string &name) : Lattice<T>(name) {}
	MaxLattice(const T &e) : Lattice<T>(e) {}
	explicit MaxLattice(const std::string &name, const T &e) : Lattice<T>(name, e) {}
	MaxLattice(const MaxLattice &other) : Lattice<T>(other) {}

	BoolLattice gt(const T &n) const{
		if (this->element_ > n) return BoolLattice(true);
		else return BoolLattice(false);
	}
	BoolLattice gt_eq(const T &n) const{
		if (this->element_ >= n) return BoolLattice(true);
		else return BoolLattice(false);
	}
	MaxLattice<T> add(const T &n) const{
		return MaxLattice<T>(this->element_ + n);
	}
	MaxLattice<T> subtract(const T &n) const{
		return MaxLattice<T>(this->element_ - n);
	}
};

}  // namespace fluent

#endif  // FLUENT_MAX_LATTICE_H_