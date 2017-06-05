#ifndef FLUENT_STDOUT_H_
#define FLUENT_STDOUT_H_

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <type_traits>
#include <utility>

#include "range/v3/all.hpp"

#include "common/macros.h"
#include "common/type_traits.h"

namespace fluent {

class Stdout {
 public:
  Stdout() {}
  DISALLOW_COPY_AND_ASSIGN(Stdout);

  const std::string& Name() const {
    static const std::string name = "stdout";
    return name;
  }

  void Merge(const std::set<std::tuple<std::string>>& ts) {
    for (const std::tuple<std::string>& t : ts) {
      std::cout << std::get<0>(t) << std::endl;
    }
  }

  void DeferredMerge(const std::set<std::tuple<std::string>>& ts) {
    deferred_merge_.insert(ts.begin(), ts.end());
  }

  std::set<std::tuple<std::string>> Tick() {
    for (const std::tuple<std::string>& t : deferred_merge_) {
      std::cout << std::get<0>(t) << std::endl;
    }
    deferred_merge_.clear();

    // TODO(mwhittaker): We could buffer lines written to stdout and return
    // them here to be reclaimed.
    return {};
  }

 private:
  std::set<std::tuple<std::string>> deferred_merge_;
};

}  // namespace fluent

#endif  // FLUENT_STDOUT_H_
