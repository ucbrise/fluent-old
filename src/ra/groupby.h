#ifndef RA_GROUPBY_H_
#define RA_GROUPBY_H_

#include <type_traits>
#include <utility>
#include <map>

#include <iostream>

#include "range/v3/all.hpp"

#include "ra/hash_fn.h"
namespace fluent {
namespace ra {

template <typename PhysicalChild, std::size_t... Is>
class PhysicalGroupBy {

  typedef ranges::iterator_value_t<ranges::range_iterator_t<typename std::result_of_t<decltype(&PhysicalChild::ToRange)(PhysicalChild)>>> valuetype;
  typedef decltype(std::tuple_cat(std::make_tuple(std::get<Is>(std::declval<valuetype>()))...)) keytype;

 public:
  PhysicalGroupBy(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    auto projection = [](const auto& t) { return std::tuple_cat(std::make_tuple(std::get<Is>(t))...); };
    mp_ = hs_(child_.ToRange(), projection);
    return ranges::view::all(mp_) 
      | ranges::view::transform([](const auto& pair) {
        return ranges::view::all(pair.second); 
        });
  }

 private:
  PhysicalChild child_;
  hash_fn hs_;
  std::map<keytype, std::vector<valuetype>> mp_;
};

template <std::size_t... Is, typename PhysicalChild>
PhysicalGroupBy<typename std::decay<PhysicalChild>::type, Is...>
make_physical_groupby(PhysicalChild&& child) {
  return PhysicalGroupBy<typename std::decay<PhysicalChild>::type, Is...>(
      std::forward<PhysicalChild>(child));
}

template <typename LogicalChild, std::size_t... Is>
class GroupBy {
 public:
  explicit GroupBy(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const {
    return make_physical_groupby<Is...>(child_.ToPhysical());
  }

 private:
  const LogicalChild child_;
};

template <std::size_t... Is, typename LogicalChild>
GroupBy<typename std::decay<LogicalChild>::type, Is...> make_groupby(
    LogicalChild&& child) {
  return GroupBy<typename std::decay<LogicalChild>::type, Is...>(
      std::forward<LogicalChild>(child));
}

template <std::size_t... Is>
struct GroupByPipe {};

template <size_t... Is>
GroupByPipe<Is...> groupby() {
  return {};
}

template <std::size_t... Is, typename LogicalChild>
GroupBy<typename std::decay<LogicalChild>::type, Is...> operator|(
    LogicalChild&& child, GroupByPipe<Is...>) {
  return make_groupby<Is...>(std::forward<LogicalChild&&>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_GROUPBY_H_
