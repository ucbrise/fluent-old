#ifndef TESTING_TEST_UTIL_H_
#define TESTING_TEST_UTIL_H_

#include "gtest/gtest.h"
#include "range/v3/all.hpp"

namespace fluent {

// `ExpectRngsEqual(x, y)` uses `EXPECT_EQ` to check that
//   (a) `x` and `y` have the same length and
//   (b) `x` and `y` are pairwise equal.
template <typename Rng1, typename Rng2>
void ExpectRngsEqual(Rng1 actual, Rng2 expected) {
  auto a_iter = ranges::begin(actual);
  auto a_end = ranges::end(actual);
  auto e_iter = ranges::begin(expected);
  auto e_end = ranges::end(expected);
  while (a_iter != a_end && e_iter != e_end) {
    EXPECT_EQ(*a_iter, *e_iter);
    ++a_iter;
    ++e_iter;
  }
  EXPECT_EQ(a_iter, a_end);
  EXPECT_EQ(e_iter, e_end);
}

}  // namespace fluent

#endif  // TESTING_TEST_UTIL_H_
