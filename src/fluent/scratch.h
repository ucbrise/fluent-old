#ifndef FLUENT_SCRATCH_H_
#define FLUENT_SCRATCH_H_

#include <algorithm>
#include <set>
#include <string>
#include <tuple>

#include "common/type_traits.h"
#include "ra/iterable.h"

namespace fluent {

template <typename... Ts>
class Scratch {
 public:
  explicit Scratch(std::string name) : name_(std::move(name)) {}

  const std::string& Name() const { return name_; }

  const std::set<std::tuple<Ts...>>& Get() const { return ts_; }

  ra::Iterable<std::set<std::tuple<Ts...>>> Iterable() const {
    return ra::make_iterable(name_, &ts_);
  }

  void Merge(const std::set<std::tuple<Ts...>>& ts) {
    ts_.insert(ts.begin(), ts.end());
  }

  std::set<std::tuple<Ts...>> Tick() {
    std::set<std::tuple<Ts...>> ts;
    std::swap(ts, ts_);
    return ts;
  }

 private:
  const std::string name_;
  std::set<std::tuple<Ts...>> ts_;
};

}  // namespace fluent

#endif  // FLUENT_SCRATCH_H_
