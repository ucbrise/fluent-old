#ifndef EXAMPLE_COUNT_H_
#define EXAMPLE_COUNT_H_

#include "range/v3/all.hpp"

namespace example {

// `count(xs)` returns the number of elements in `xs`.
template <typename Rng, typename I = ranges::range_iterator_t<Rng>,
          CONCEPT_REQUIRES_(ranges::InputRange<Rng>())>
ranges::iterator_difference_t<I> Count(const Rng& rng) {
  ranges::iterator_difference_t<I> c = 0;
  for (auto i = ranges::begin(rng); i != ranges::end(rng); ++i) {
    c++;
  }
  return c;
}

}  // namespace example

#endif  // EXAMPLE_COUNT_H_
