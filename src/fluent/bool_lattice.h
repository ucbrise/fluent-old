#ifndef FLUENT_BOOL_LATTICE_H_
#define FLUENT_BOOL_LATTICE_H_

#include "fluent/base_lattice.h"

namespace fluent {

class BoolLattice : public Lattice<bool> {
protected:
	void do_merge(const bool &e) {
		// we need 'this' because this is a templated class
		// 'this' makes the name dependent so that we can access the base definition
		this->element_ |= e;
	}
public:
	explicit BoolLattice() : Lattice<bool>() {}
	explicit BoolLattice(const std::string &name) : Lattice<bool>(name) {}
	BoolLattice(const bool &e) : Lattice<bool>(e) {}
	explicit BoolLattice(const std::string &name, const bool &e) : Lattice<bool>(name, e) {}
	BoolLattice(const BoolLattice &other) : Lattice<bool>(other) {}
};

}  // namespace fluent

#endif  // FLUENT_BOOL_LATTICE_H_