#ifndef FLUENT_SCRATCH_H_
#define FLUENT_SCRATCH_H_

#include <set>
#include <string>
#include <tuple>

#include "ra/iterable.h"
#include "ra/ra_util.h"

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

  template <typename RA>
  void Merge(const RA& ra) {
    ra::BufferRaInto(ra, &ts_);
  }

  void Tick() { ts_.clear(); }

 private:
  const std::string name_;
  std::set<std::tuple<Ts...>> ts_;
};

}  // namespace fluent

#endif  // FLUENT_SCRATCH_H_
