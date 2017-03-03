#ifndef FLUENT_SCRATCH_H_
#define FLUENT_SCRATCH_H_

#include <set>
#include <string>
#include <tuple>

#include "ra/iterable.h"

namespace fluent {

template <typename... Ts>
class Scratch {
 public:
  explicit Scratch(std::string name) : name_(std::move(name)) {}

  const std::string& Name() const { return name_; }

  const std::set<std::tuple<Ts...>>& Get() const { return ts_; }

  ra::Iterable<std::set<std::tuple<Ts...>>> Iterable() const {
    return ra::make_iterable(&ts_);
  }

  // TODO(mwhittaker): This method definition is copied verbatim from the
  // implementation of table.h. Refactor things so we don't have to duplicate
  // the code.
  template <typename RA>
  void Merge(const RA& ra) {
    // If `query` includes an iterable over `ts_`, then inserting into `ts_`
    // might invalidate the iterator. Thus, we first write into a temprorary
    // vector and then copy the contents of the vector into the `ts_`.
    auto physical = ra.ToPhysical();
    auto rng = physical.ToRange();
    auto buf = rng | ranges::to_<std::set<std::tuple<Ts...>>>();
    auto begin = std::make_move_iterator(std::begin(buf));
    auto end = std::make_move_iterator(std::end(buf));
    ts_.insert(begin, end);
  }

  void Tick() { ts_.clear(); }

 private:
  const std::string name_;
  std::set<std::tuple<Ts...>> ts_;
};

}  // namespace fluent

#endif  // FLUENT_SCRATCH_H_
