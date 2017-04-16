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

template <typename K, typename V>
class PhysicalMapIterable {
 public:
  explicit PhysicalMapIterable(const std::unordered_map<K, V>* iterable) : iterable_(iterable) {}

  auto ToRange() {
    return ToRange(std::make_tuple());
  }

  template <typename T, typename VT = V>
  auto ToRange(const T t, typename std::enable_if<fluent::has_Iterable<VT>::value>::type* dummy = nullptr) {
  	(void) dummy;
  	//std::cout << "has iterable\n";
  	return ranges::view::all(*iterable_) | ranges::view::for_each([t](const auto& l) {
  		return ranges::yield_from(l.second.Iterable().ToPhysical().ToRange(std::tuple_cat(t, std::make_tuple(l.first))));
  	}); 
  }

  template <typename T, typename VT = V>
  auto ToRange(const T t, typename std::enable_if<!(fluent::has_Iterable<VT>::value)>::type* dummy = nullptr) { 
  	(void) dummy;
  	//std::cout << "no iterable\n";
  	return ranges::view::all(*iterable_) | ranges::view::transform([t](const auto& l) {
  		return std::tuple_cat(t, std::make_tuple(l.first, l.second));
  	}); 
  }

 private:
  const std::unordered_map<K, V>* iterable_;
};

template <typename K, typename V>
PhysicalMapIterable<K, V> make_physical_map_iterable(const std::unordered_map<K, V>* iterable) {
  return PhysicalMapIterable<K, V>(iterable);
}

template <typename K, typename V>
class MapIterable {
 public:
  using column_types = typename TupleToTypeList<typename ranges::range_value_t<typename std::decay<decltype(std::declval<PhysicalMapIterable<K, V>>().ToRange(std::make_tuple()))>::type>>::type;

  explicit MapIterable(const std::unordered_map<K, V>* iterable) : iterable_(iterable) {}

  auto ToPhysical() const { return make_physical_map_iterable(iterable_); }

 private:
  const std::unordered_map<K, V>* iterable_;
};

template <typename K, typename V>
MapIterable<K, V> make_map_iterable(const std::unordered_map<K, V>* iterable) {
  return MapIterable<K, V>(iterable);
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_MAP_ITERABLE_H_
