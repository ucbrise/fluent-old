#ifndef FLUENT_MAX_LATTICE_H_
#define FLUENT_MAX_LATTICE_H_

#include "fluent/base_lattice.h"

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
	explicit MaxLattice(const MaxLattice &other) : Lattice<T>(other) {}
};

}  // namespace fluent

#endif  // FLUENT_MAX_LATTICE_H_