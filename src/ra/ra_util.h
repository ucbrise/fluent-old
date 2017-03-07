#ifndef RA_RA_UTIL_H_
#define RA_RA_UTIL_H_

#include <iterator>
#include <set>
#include <tuple>
#include <vector>

#include "range/v3/all.hpp"

namespace fluent {
namespace ra {

// `BufferRaInto(ra, s)` collects the result of evaluating the relational
// algebra expression `ra` into a intermediate buffer and then *moves* the
// contents of the buffer into `s`. An example:
//
//   std::set<tuple<int, int>> t = {{3, 3}, {4, 4}, {5, 5}};
//   std::set<tuple<int, int>> s = {{1, 1}, {2, 2}, {3, 3}};
//   BufferRaInto(ra::make_iterable(&t), &s);
//   // s contains {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}}
//
// Note that because the results of `ra` are stored into an intermediate buffer
// before being moved into `s`, it is okay if `ra` involves an iterator over
// `s`.
template <typename RA, typename... Ts>
void BufferRaInto(const RA& ra, std::set<std::tuple<Ts...>>* s) {
  // If `query` includes an iterable over `ts_`, then inserting into `ts_`
  // might invalidate the iterator. Thus, we first write into a temprorary
  // vector and then copy the contents of the vector into the `ts_`.
  auto physical = ra.ToPhysical();
  auto rng = physical.ToRange();
  auto buf = rng | ranges::to_<std::vector<std::tuple<Ts...>>>();
  auto begin = std::make_move_iterator(std::begin(buf));
  auto end = std::make_move_iterator(std::end(buf));
  s->insert(begin, end);
}

// `StreamRaInto(ra, s)` streams the results of evaluating the relational
// algebra expression `ra` into `s`. Unlike with BufferRaInto, results are not
// buffered. Note that because the results of `ra` are *not* stored into an
// intermediate buffer before being moved into `s`, it is *not* okay if `ra`
// involves an iterator over `s`. If `ra` contains an iterator over `s`, then
// those iterators would be invalidated by streaming into `s`.
template <typename RA, typename... Ts>
void StreamRaInto(const RA& ra, std::vector<std::tuple<Ts...>>* s) {
  auto physical = ra.ToPhysical();
  auto rng = physical.ToRange();
  for (auto iter = ranges::begin(rng); iter != ranges::end(rng); ++iter) {
    s->insert(s->end(), std::move(*iter));
  }
}

}  // namespace ra
}  // namespace fluent

#endif  // RA_RA_UTIL_H_
