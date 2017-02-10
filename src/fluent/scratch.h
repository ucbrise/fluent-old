#ifndef FLUENT_SCRATCH_H_
#define FLUENT_SCRATCH_H_

#include "fluent/collection.h"
#include "fluent/rule_tags.h"

namespace fluent {

template <typename... Ts>
class Scratch : public Collection<Ts...> {
 public:
  explicit Scratch(const std::string& name) : Collection<Ts...>(name) {}

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
    this->MutableGet().insert(begin, end);
  }

  template <typename Rhs>
  std::tuple<Scratch<Ts...>*, MergeTag, typename std::decay<Rhs>::type>
  operator<=(Rhs&& rhs) {
    return {this, MergeTag(), std::forward<Rhs>(rhs)};
  }

  void Tick() override { this->MutableGet().clear(); }
};

}  // namespace fluent

#endif  // FLUENT_SCRATCH_H_
