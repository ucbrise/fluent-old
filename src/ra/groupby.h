#ifndef RA_GroupBy_H_
#define RA_GroupBy_H_

#include <type_traits>
#include <utility>
#include <map>

#include <iostream>

#include "range/v3/all.hpp"

namespace fluent {
namespace ra {

struct hash_fn {
  template <typename I, typename S, typename P,
            typename V = ranges::iterator_value_t<I>,
            typename R = ranges::iterator_reference_t<I>,
            typename Projected = std::result_of_t<P(R)>,
            CONCEPT_REQUIRES_(ranges::InputIterator<I>() && ranges::Sentinel<S, I>())>
  std::map<Projected, std::vector<V>> operator()(I begin, S end,
                                                           P p) const {
    std::map<Projected, std::vector<ranges::iterator_value_t<I>>> t;
    for (; begin != end; ++begin) {
      t[p(*begin)].push_back(*begin);
    }
    return t;
  }

  template <typename Rng, typename P, typename I = ranges::range_iterator_t<Rng>,
            typename V = ranges::iterator_value_t<I>,
            typename R = ranges::iterator_reference_t<I>,
            typename Projected = std::result_of_t<P(R)>,
            CONCEPT_REQUIRES_(ranges::InputRange<Rng>())>
  std::map<Projected, std::vector<V>> operator()(Rng &&rng,
                                                           P p) const {
    return (*this)(begin(rng), end(rng), std::move(p));
  }
};

template <typename PhysicalChild, std::size_t... Is>
class PhysicalGroupBy {

  typedef ranges::iterator_value_t<ranges::range_iterator_t<typename std::result_of_t<decltype(&PhysicalChild::ToRange)(PhysicalChild)>>> valuetype;
  //typedef typename std::tuple_element<Is, valuetype>::type keytype;
  typedef decltype(std::tuple_cat(std::make_tuple(std::get<Is>(std::declval<valuetype>()))...)) keytype;

 public:
  PhysicalGroupBy(PhysicalChild child) : child_(std::move(child)) {}

  auto ToRange() {
    auto project = [](const auto& t) { return std::tuple_cat(std::make_tuple(std::get<Is>(t))...); };
    mp = hs_(child_.ToRange(), project);
    return ranges::view::all(mp) 
      | ranges::view::transform([](const auto& pair) {
        return ranges::view::all(pair.second); 
        });
  }

 private:
  PhysicalChild child_;
  hash_fn hs_;
  std::map<keytype, std::vector<valuetype>> mp;
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

#endif  // RA_GroupBy_H_
