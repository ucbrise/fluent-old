#ifndef RA_ITERABLE_H_
#define RA_ITERABLE_H_

#include <string>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/hash_util.h"
#include "common/type_list.h"
#include "common/type_traits.h"
#include "ra/lineaged_tuple.h"

namespace fluent {
namespace ra {

template <typename T, template <typename> class Hash>
class PhysicalIterable {
 public:
  PhysicalIterable(const std::string* name, const T* iterable)
      : name_(name), iterable_(iterable) {}

  auto ToRange() {
    return ranges::view::all(*iterable_) |
           ranges::view::transform([this](const auto& t) {
             using t_type = typename std::decay<decltype(t)>::type;
             return make_lineaged_tuple({{*name_, Hash<t_type>()(t)}}, t);
           });
  }

 private:
  const std::string* const name_;
  const T* iterable_;
};

template <template <typename> class Hash, typename T>
PhysicalIterable<T, Hash> make_physical_iterable(const std::string* name,
                                                 const T* iterable) {
  return {name, iterable};
}

template <typename T, template <typename> class Hash = Hash>
class Iterable {
 public:
  static_assert(IsTuple<typename T::value_type>::value,
                "Iterables must contain tuples.");
  using column_types = typename TupleToTypeList<typename T::value_type>::type;

  Iterable(std::string name, const T* iterable)
      : name_(std::move(name)), iterable_(iterable) {}

  auto ToPhysical() const {
    return make_physical_iterable<Hash>(&name_, iterable_);
  }

  std::string ToDebugString() const { return name_; }

 private:
  const std::string name_;
  const T* iterable_;
};

template <typename T>
Iterable<T> make_iterable(std::string name, const T* iterable) {
  return {std::move(name), iterable};
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_ITERABLE_H_
