#ifndef FLUENT_TABLE_H_
#define FLUENT_TABLE_H_

#include <algorithm>
#include <iterator>
#include <set>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "fluent/collection.h"
#include "fluent/rule_tags.h"

namespace fluent {

template <typename... Ts>
class Table : public Collection<Ts...> {
 public:
  explicit Table(const std::string& name) : Collection<Ts...>(name) {}

  template <typename RA>
  void Merge(const RA& ra) {
    BufferRaInto(ra, &this->MutableGet());
  }

  template <typename RA>
  void DeferredMerge(const RA& ra) {
    StreamRaInto(ra, &deferred_merge_);
  }

  template <typename RA>
  void DeferredDelete(const RA& ra) {
    StreamRaInto(ra, &deferred_delete_);
  }

  template <typename Rhs>
  std::tuple<Table<Ts...>*, MergeTag, typename std::decay<Rhs>::type>
  operator<=(Rhs&& rhs) {
    return {this, MergeTag(), std::forward<Rhs>(rhs)};
  }

  template <typename Rhs>
  std::tuple<Table<Ts...>*, DeferredMergeTag, typename std::decay<Rhs>::type>
  operator+=(Rhs&& rhs) {
    return {this, DeferredMergeTag(), std::forward<Rhs>(rhs)};
  }

  template <typename Rhs>
  std::tuple<Table<Ts...>*, DeferredDeleteTag, typename std::decay<Rhs>::type>
  operator-=(Rhs&& rhs) {
    return {this, DeferredDeleteTag(), std::forward<Rhs>(rhs)};
  }

  void Tick() override {
    this->MutableGet().insert(
        std::make_move_iterator(std::begin(deferred_merge_)),
        std::make_move_iterator(std::end(deferred_merge_)));
    for (const std::tuple<Ts...>& t : deferred_delete_) {
      this->MutableGet().erase(t);
    }

    deferred_merge_.clear();
    deferred_delete_.clear();
  }

 private:
  template <typename RA>
  void BufferRaInto(const RA& ra, std::set<std::tuple<Ts...>>* s) {
    // If `query` includes an iterable over `ts_`, then inserting into `ts_`
    // might invalidate the iterator. Thus, we first write into a temprorary
    // vector and then copy the contents of the vector into the `ts_`.
    auto physical = ra.ToPhysical();
    auto rng = physical.ToRange();
    auto buf = rng | ranges::to_<std::set<std::tuple<Ts...>>>();
    auto begin = std::make_move_iterator(std::begin(buf));
    auto end = std::make_move_iterator(std::end(buf));
    s->insert(begin, end);
  }

  template <typename RA>
  void StreamRaInto(const RA& ra, std::set<std::tuple<Ts...>>* s) {
    auto physical = ra.ToPhysical();
    auto rng = physical.ToRange();
    for (auto iter = ranges::begin(rng); iter != ranges::end(rng); ++iter) {
      s->insert(std::move(*iter));
    }
  }

  std::set<std::tuple<Ts...>> deferred_merge_;
  std::set<std::tuple<Ts...>> deferred_delete_;
};

}  // namespace fluent

#endif  // FLUENT_TABLE_H_
