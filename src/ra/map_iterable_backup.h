#ifndef RA_MAP_ITERABLE_H_
#define RA_MAP_ITERABLE_H_

#include <iostream>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "range/v3/all.hpp"

#include "common/function_traits.h"
#include "common/type_list.h"

namespace fluent {
namespace ra {

template <typename K, typename V, typename T>
class PhysicalMapIterable {
 public:
  explicit PhysicalMapIterable(const std::unordered_map<K, V>* iterable, const T t) : iterable_(iterable), t_(t) {}

  //typename std::enable_if<fluent::has_Iterable<V>::value, auto>::type
  //template<typename std::enable_if<fluent::has_Iterable<V>::value>::type>
  template <typename VT = V>
  auto ToRange(typename std::enable_if<fluent::has_Iterable<VT>::value>::type* dummy = nullptr) {
  	(void) dummy;
  	std::cout << "has iterable\n";
  	return ranges::view::all(*iterable_) | ranges::view::for_each([this](const auto& l) {
  		std::cout << std::get<0>(std::make_tuple(l.first)) << "\n";
  		auto inner = l.second.Iterable(std::tuple_cat(this->t_, std::make_tuple(l.first)));
  		std::cout << std::get<0>(inner.t_) << "\n";
  		return ranges::yield_from(inner.ToPhysical().ToRange());
  	}); 
  }

  //typename std::enable_if<!fluent::has_Iterable<V>::value, auto>::type
  //template<typename std::enable_if<!fluent::has_Iterable<V>::value>::type>
  template <typename VT = V>
  auto ToRange(typename std::enable_if<!(fluent::has_Iterable<VT>::value)>::type* dummy = nullptr) { 
  	(void) dummy;
  	std::cout << "no iterable\n";
  	return ranges::view::all(*iterable_) | ranges::view::transform([this](const auto& l) {
  		std::cout << std::get<0>(this->t_) << "\n";
  		return std::tuple_cat(this->t_, std::make_tuple(l.first, l.second));
  	}); 
  }

 //private:
  const std::unordered_map<K, V>* iterable_;
  const T t_;
};

template <typename K, typename V, typename T>
PhysicalMapIterable<K, V, T> make_physical_map_iterable(const std::unordered_map<K, V>* iterable, const T t) {
  return PhysicalMapIterable<K, V, T>(iterable, t);
}

template <typename K, typename V, typename T>
class MapIterable {
 public:
  //using column_types = typename ranges::range_value_t<typename std::decay<decltype(std::declval<PhysicalMapIterable<K, V, T>>().ToRange())>::type>;

  explicit MapIterable(const std::unordered_map<K, V>* iterable, const T t) : iterable_(iterable), t_(t) {}

  auto ToPhysical() const { return make_physical_map_iterable(iterable_, t_); }

 //private:
  const std::unordered_map<K, V>* iterable_;
  const T t_;
};

template <typename K, typename V, typename T>
MapIterable<K, V, T> make_map_iterable(const std::unordered_map<K, V>* iterable, const T t) {
  return MapIterable<K, V, T>(iterable, t);
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_MAP_ITERABLE_H_
