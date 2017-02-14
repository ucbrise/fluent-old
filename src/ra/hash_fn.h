#ifndef RA_HASHFN_H_
#define RA_HASHFN_H_

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

}  // namespace ra
}  // namespace fluent

#endif  // RA_HASHFN_H_
