#ifndef FLUENT_STDOUT_H_
#define FLUENT_STDOUT_H_

#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/macros.h"
#include "ra/ra_util.h"

namespace fluent {

class Stdout {
 public:
  Stdout() {}
  DISALLOW_COPY_AND_ASSIGN(Stdout);

  const std::string& Name() const {
    static const std::string name = "stdout";
    return name;
  }

  template <typename RA>
  void Merge(const RA& ra) {
    auto physical = ra.ToPhysical();
    auto rng = physical.ToRange();
    for (auto iter = ranges::begin(rng); iter != ranges::end(rng); ++iter) {
      using value_type =
          typename std::decay<decltype(std::get<0>(*iter))>::type;
      static_assert(std::is_same<value_type, std::string>::value,
                    "Stdout can only merge strings.");
      std::cout << std::get<0>(*iter) << std::endl;
    }
  }

  template <typename RA>
  void DeferredMerge(const RA& ra) {
    ra::StreamRaInto(ra, &deferred_merge_);
  }

  void Tick() {
    for (const std::tuple<std::string>& t : deferred_merge_) {
      std::cout << std::get<0>(t) << std::endl;
    }
    deferred_merge_.clear();
  }

 private:
  std::set<std::tuple<std::string>> deferred_merge_;
};

}  // namespace fluent

#endif  // FLUENT_STDOUT_H_
