#ifndef RELALG_PROJECT_H_
#define RELALG_PROJECT_H_

#include "range/v3/all.hpp"

namespace fluent {

// A relational projection. Project is nothing more than a synonym of
// transform! See http://bit.ly/2jIgXOH to see exactly how `transform` is
// defined.
RANGES_INLINE_VARIABLE(ranges::view::view<ranges::view::transform_fn>, project)

}  // namespace fluent

#endif  // RELALG_PROJECT_H_
