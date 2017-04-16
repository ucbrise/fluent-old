#ifndef FLUENT_MAX_LATTICE_H_
#define FLUENT_MAX_LATTICE_H_

#include "fluent/base_lattice.h"
#include "fluent/bool_lattice.h"

namespace fluent {

template <typename T>
class MaxLattice : public Lattice<MaxLattice<T>, T> {
public:
	MaxLattice() = default;
	MaxLattice(const std::string &name) : name_(name), element_() {}
	MaxLattice(const T &e) : name_(""), element_(e) {}
	MaxLattice(const std::string &name, const T &e) : name_(name), element_(e) {}
	MaxLattice(const MaxLattice<T> &l) = default;
	MaxLattice& operator=(const MaxLattice<T>& l) = default;

	const std::string& Name() const override { return name_; }
	const T& Reveal() const override { return element_; }
  	void merge(const MaxLattice<T>& l) override { element_ = std::max(element_, l.element_); }
  	void merge(const T& t) override { element_ = std::max(element_, t); }

	template <typename RA>
	typename std::enable_if<!(std::is_base_of<Lattice<MaxLattice<T>, T>, RA>::value)>::type
	Merge(const RA& ra) {
		auto buf = ra::MergeRaInto<RA>(ra);
		auto begin = std::make_move_iterator(std::begin(buf));
		auto end = std::make_move_iterator(std::end(buf));
		for (auto it = begin; it != end; it++) {
		  merge(std::get<0>(*it));
		}
	}

	template <typename L>
	typename std::enable_if<(std::is_base_of<Lattice<MaxLattice<T>, T>, L>::value)>::type
	Merge(const L& l) {
		merge(l);
	}

	BoolLattice gt(const T &n) const{
		return BoolLattice(element_ > n);
	}
	BoolLattice gt_eq(const T &n) const{
		return BoolLattice(element_ >= n);
	}
	MaxLattice<T> add(const T &n) const{
		return MaxLattice<T>(this->element_ + n);
	}
	MaxLattice<T> subtract(const T &n) const{
		return MaxLattice<T>(this->element_ - n);
	}

	friend bool operator<(const MaxLattice<T>& lhs, const MaxLattice<T>& rhs) {
		return lhs.element_ < rhs.element_;
	}
	// friend bool operator<=(const MaxLattice<T>& lhs, const MaxLattice<T>& rhs) {
	// 	return lhs.element_ <= rhs.element_;
	// }
	friend bool operator>(const MaxLattice<T>& lhs, const MaxLattice<T>& rhs) {
		return lhs.element_ > rhs.element_;
	}
	friend bool operator>=(const MaxLattice<T>& lhs, const MaxLattice<T>& rhs) {
		return lhs.element_ >= rhs.element_;
	}
	friend bool operator==(const MaxLattice<T>& lhs, const MaxLattice<T>& rhs) {
		return lhs.element_ == rhs.element_;
	}
	friend bool operator!=(const MaxLattice<T>& lhs, const MaxLattice<T>& rhs) {
		return lhs.element_ != rhs.element_;
	}

private:
	std::string name_;
	T element_;
};

}  // namespace fluent

#endif  // FLUENT_MAX_LATTICE_H_