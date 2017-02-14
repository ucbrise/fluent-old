#ifndef RA_GROUPSUM_H_
#define RA_GROUPSUM_H_

#include <type_traits>
#include <utility>

#include <iostream>

#include "range/v3/all.hpp"

/* This operator should only be used following ra::groupby */

namespace fluent {
namespace ra {

template <typename PhysicalChild, std::size_t Is>
class PhysicalGroupSum {

  typedef ranges::iterator_value_t<ranges::range_iterator_t<typename std::result_of_t<decltype(&PhysicalChild::ToRange)(PhysicalChild)>>> rngtype;
  typedef ranges::iterator_value_t<ranges::range_iterator_t<rngtype>> valuetype;

 public:
  PhysicalGroupSum(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    return child_.ToRange() | ranges::view::transform([](const auto& rng) {
          std::vector<valuetype> v = rng | ranges::to_<std::vector<valuetype>>();
          decltype(std::get<Is>(std::declval<valuetype>())) s = 0;
          for (auto& t: v)
            s += std::get<Is>(t);
          return std::make_tuple(s);
       });
  }

 private:
  PhysicalChild child_;
};

template <std::size_t Is, typename PhysicalChild>
PhysicalGroupSum<typename std::decay<PhysicalChild>::type, Is>
make_physical_groupsum(PhysicalChild&& child) {
  return PhysicalGroupSum<typename std::decay<PhysicalChild>::type, Is>(
      std::forward<PhysicalChild>(child));
}

template <typename LogicalChild, std::size_t Is>
class GroupSum {
 public:
  explicit GroupSum(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const {
    return make_physical_groupsum<Is>(child_.ToPhysical());
  }

 private:
  const LogicalChild child_;
};

template <std::size_t Is, typename LogicalChild>
GroupSum<typename std::decay<LogicalChild>::type, Is> make_groupsum(
    LogicalChild&& child) {
  return GroupSum<typename std::decay<LogicalChild>::type, Is>(
      std::forward<LogicalChild>(child));
}

template <std::size_t Is>
struct GroupSumPipe {};

template <size_t Is>
GroupSumPipe<Is> groupsum() {
  return {};
}

template <std::size_t Is, typename LogicalChild>
GroupSum<typename std::decay<LogicalChild>::type, Is> operator|(
    LogicalChild&& child, GroupSumPipe<Is>) {
  return make_groupsum<Is>(std::forward<LogicalChild&&>(child));
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_GROUPSUM_H_
