#ifndef FLUENT_MIN_LATTICE_H_
#define FLUENT_MIN_LATTICE_H_

#include "fluent/base_lattice.h"
#include "fluent/bool_lattice.h"

namespace fluent {

template <typename T>
class MinLattice : public Lattice<MinLattice<T>, T> {
public:
	MinLattice() = default;
	MinLattice(const std::string &name) : name_(name), element_() {}
	MinLattice(const T &e) : name_(""), element_(e) {}
	MinLattice(const std::string &name, const T &e) : name_(name), element_(e) {}
	MinLattice(const MinLattice<T> &l) = default;
	MinLattice& operator=(const MinLattice<T>& l) = default;

	const std::string& Name() const override { return name_; }
	const T& Reveal() const override { return element_; }
  	void merge(const MinLattice<T>& l) override { element_ = std::min(element_, l.element_); }
  	void merge(const T& t) override { element_ = std::min(element_, t); }

	template <typename RA>
	typename std::enable_if<!(std::is_base_of<Lattice<MinLattice<T>, T>, RA>::value)>::type
	Merge(const RA& ra) {
		auto buf = ra::MergeRaInto<RA>(ra);
		auto begin = std::make_move_iterator(std::begin(buf));
		auto end = std::make_move_iterator(std::end(buf));
		for (auto it = begin; it != end; it++) {
		  merge(std::get<0>(*it));
		}
	}

	template <typename L>
	typename std::enable_if<(std::is_base_of<Lattice<MinLattice<T>, T>, L>::value)>::type
	Merge(const L& l) {
		merge(l);
	}

	BoolLattice lt(T n) const{
		return BoolLattice(element_ < n);
	}
	BoolLattice lt_eq(T n) const{
		return BoolLattice(element_ <= n);
	}
	MinLattice<T> add(T n) const{
		return MinLattice<T>(this->element_ + n);
	}
	MinLattice<T> subtract(T n) const{
		return MinLattice<T>(this->element_ - n);
	}

	friend bool operator<(const MinLattice<T>& lhs, const MinLattice<T>& rhs) {
		return lhs.element_ < rhs.element_;
	}
	// friend bool operator<=(const MinLattice<T>& lhs, const MinLattice<T>& rhs) {
	// 	return lhs.element_ <= rhs.element_;
	// }
	friend bool operator>(const MinLattice<T>& lhs, const MinLattice<T>& rhs) {
		return lhs.element_ > rhs.element_;
	}
	friend bool operator>=(const MinLattice<T>& lhs, const MinLattice<T>& rhs) {
		return lhs.element_ >= rhs.element_;
	}
	friend bool operator==(const MinLattice<T>& lhs, const MinLattice<T>& rhs) {
		return lhs.element_ == rhs.element_;
	}
	friend bool operator!=(const MinLattice<T>& lhs, const MinLattice<T>& rhs) {
		return lhs.element_ != rhs.element_;
	}

private:
	std::string name_;
	T element_;
};

}  // namespace fluent

#endif  // FLUENT_MIN_LATTICE_H_