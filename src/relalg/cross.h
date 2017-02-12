#ifndef RELALG_CROSS_H_
#define RELALG_CROSS_H_

#include <utility>

#include "range/v3/all.hpp"

namespace fluent {

// A relational cross product. This view more or less performs the following
// psuedocode:
//
//   def cross(lefts, rights):
//     for left in lefts:
//       for right in rights:
//         yield make_pair(left, right)
//
// The cross product can be combined with a select to implement a cross join.
struct cross_fn {
  template <typename Rng1, typename Rng2,
            CONCEPT_REQUIRES_(ranges::Range<Rng1>() && ranges::Range<Rng2>())>
  auto operator()(Rng1&& lefts, Rng2&& rights) const {
    return ranges::view::for_each(
        lefts, [&rights](ranges::range_reference_t<Rng1> left) {
          return ranges::yield_from(
              rights | ranges::view::transform(
                           [&left](ranges::range_reference_t<Rng2> right) {
                             return std::make_pair(left, right);
                           }));
        });
  }
};

RANGES_INLINE_VARIABLE(cross_fn, cross)

}  // namespace fluent

#endif  // RELALG_CROSS_H_
