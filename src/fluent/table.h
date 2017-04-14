#ifndef FLUENT_TABLE_H_
#define FLUENT_TABLE_H_

#include <algorithm>
#include <array>
#include <iterator>
#include <set>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/macros.h"
#include "common/type_traits.h"
#include "ra/iterable.h"

namespace fluent {

template <typename... Ts>
class Table {
 public:
  template <typename... Strings>
  Table(std::string name, std::array<std::string, sizeof...(Ts)> column_names)
      : name_(std::move(name)), column_names_(std::move(column_names)) {}
  Table(Table&&) = default;
  Table& operator=(Table&&) = default;
  DISALLOW_COPY_AND_ASSIGN(Table);

  const std::string& Name() const { return name_; }

  const std::array<std::string, sizeof...(Ts)>& ColumnNames() const {
    return column_names_;
  }

  const std::set<std::tuple<Ts...>>& Get() const { return ts_; }

  ra::Iterable<std::set<std::tuple<Ts...>>> Iterable() const {
    return ra::make_iterable(name_, &ts_);
  }

  void Merge(const std::set<std::tuple<Ts...>>& ts) {
    ts_.insert(ts.begin(), ts.end());
  }

  void DeferredMerge(const std::set<std::tuple<Ts...>>& ts) {
    deferred_merge_.insert(ts.begin(), ts.end());
  }

  void DeferredDelete(const std::set<std::tuple<Ts...>>& ts) {
    deferred_delete_.insert(ts.begin(), ts.end());
  }

  std::set<std::tuple<Ts...>> Tick() {
    ts_.insert(std::begin(deferred_merge_), std::end(deferred_merge_));
    for (const std::tuple<Ts...>& t : deferred_delete_) {
      ts_.erase(t);
    }

    deferred_merge_.clear();

    std::set<std::tuple<Ts...>> ts;
    std::swap(deferred_delete_, ts);
    return ts;
  }

 private:
  const std::string name_;
  const std::array<std::string, sizeof...(Ts)> column_names_;
  std::set<std::tuple<Ts...>> ts_;
  std::set<std::tuple<Ts...>> deferred_merge_;
  std::set<std::tuple<Ts...>> deferred_delete_;
};

}  // namespace fluent

#endif  // FLUENT_TABLE_H_
