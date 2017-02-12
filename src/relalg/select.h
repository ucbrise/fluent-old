#ifndef RELALG_SELECT_H_
#define RELALG_SELECT_H_

#include "range/v3/all.hpp"

namespace fluent {

// A relational selection. Select is nothing more than a synonym of filter! See
// http://bit.ly/2jlThzU to see exactly how `filter` is defined.
RANGES_INLINE_VARIABLE(ranges::view::filter_fn, select)

}  // namespace fluent

#endif  // RELALG_SELECT_H_
