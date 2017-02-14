#ifndef RA_GroupAvg_H_
#define RA_GroupAvg_H_

#include <type_traits>
#include <utility>

#include <iostream>

#include "range/v3/all.hpp"

/* This operator should only be used following ra::groupby */

namespace fluent {
namespace ra {

template <typename PhysicalChild, std::size_t Is>
class PhysicalGroupAvg {

  typedef ranges::iterator_value_t<ranges::range_iterator_t<typename std::result_of_t<decltype(&PhysicalChild::ToRange)(PhysicalChild)>>> rngtype;
  typedef ranges::iterator_value_t<ranges::range_iterator_t<rngtype>> valuetype;
  //typedef typename std::tuple_element<Is, valuetype>::type keytype;

 public:
  PhysicalGroupAvg(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    return child_.ToRange() | ranges::view::transform([](const auto& rng) {
          auto length = ranges::size(rng);
          std::vector<valuetype> v = rng | ranges::to_<std::vector<valuetype>>();
          decltype(std::get<Is>(std::declval<valuetype>())) s = 0;
          for (auto& t: v)
            s += std::get<Is>(t);
          return std::make_tuple(s/length);
       });
  }

 private:
  PhysicalChild child_;
};

template <std::size_t Is, typename PhysicalChild>
PhysicalGroupAvg<typename std::decay<PhysicalChild>::type, Is>
make_physical_groupavg(PhysicalChild&& child) {
  return PhysicalGroupAvg<typename std::decay<PhysicalChild>::type, Is>(
      std::forward<PhysicalChild>(child));
}

template <typename LogicalChild, std::size_t Is>
class GroupAvg {
 public:
  explicit GroupAvg(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const {
    return make_physical_groupavg<Is>(child_.ToPhysical());
  }

 private:
  const LogicalChild child_;
};

template <std::size_t Is, typename LogicalChild>
GroupAvg<typename std::decay<LogicalChild>::type, Is> make_groupavg(
    LogicalChild&& child) {
  return GroupAvg<typename std::decay<LogicalChild>::type, Is>(
      std::forward<LogicalChild>(child));
}

template <std::size_t Is>
struct GroupAvgPipe {};

template <size_t Is>
GroupAvgPipe<Is> groupavg() {
  return {};
}

template <std::size_t Is, typename LogicalChild>
GroupAvg<typename std::decay<LogicalChild>::type, Is> operator|(
    LogicalChild&& child, GroupAvgPipe<Is>) {
  return make_groupavg<Is>(std::forward<LogicalChild&&>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_GroupAvg_H_
