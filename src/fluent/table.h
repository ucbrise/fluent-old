#ifndef FLUENT_TABLE_H_
#define FLUENT_TABLE_H_

#include <algorithm>
#include <iterator>
#include <set>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/type_traits.h"
#include "ra/iterable.h"
#include "ra/ra_util.h"

namespace fluent {

template <typename... Ts>
class Table {
 public:
  explicit Table(std::string name) : name_(std::move(name)) {}

  const std::string& Name() const { return name_; }

  const std::set<std::tuple<Ts...>>& Get() const { return ts_; }

  ra::Iterable<std::set<std::tuple<Ts...>>> Iterable() const {
    return ra::make_iterable(&ts_);
  }

  template <typename RA>
  void Merge(const RA& ra) {
    static_assert(!IsSet<typename std::decay<RA>::type>::value, "");
    static_assert(!IsVector<typename std::decay<RA>::type>::value, "");
    ra::BufferRaInto(ra, &ts_);
  }

  void Merge(const std::set<std::tuple<Ts...>>& ts) {
    ts_.insert(ts.begin(), ts.end());
  }

  template <typename RA>
  void DeferredMerge(const RA& ra) {
    static_assert(!IsSet<typename std::decay<RA>::type>::value, "");
    static_assert(!IsVector<typename std::decay<RA>::type>::value, "");
    ra::StreamRaInto(ra, &deferred_merge_);
  }

  void DeferredMerge(const std::set<std::tuple<Ts...>>& ts) {
    deferred_merge_.insert(ts.begin(), ts.end());
  }

  template <typename RA>
  void DeferredDelete(const RA& ra) {
    static_assert(!IsSet<typename std::decay<RA>::type>::value, "");
    static_assert(!IsVector<typename std::decay<RA>::type>::value, "");
    ra::StreamRaInto(ra, &deferred_delete_);
  }

  void DeferredDelete(const std::set<std::tuple<Ts...>>& ts) {
    deferred_delete_.insert(ts.begin(), ts.end());
  }

  void Tick() {
    ts_.insert(std::begin(deferred_merge_), std::end(deferred_merge_));
    for (const std::tuple<Ts...>& t : deferred_delete_) {
      ts_.erase(t);
    }

    deferred_merge_.clear();
    deferred_delete_.clear();
  }

 private:
  const std::string name_;
  std::set<std::tuple<Ts...>> ts_;
  std::set<std::tuple<Ts...>> deferred_merge_;
  std::set<std::tuple<Ts...>> deferred_delete_;
};

}  // namespace fluent

#endif  // FLUENT_TABLE_H_
