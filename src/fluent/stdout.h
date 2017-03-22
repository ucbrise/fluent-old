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
#include "common/type_traits.h"
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
    static_assert(!IsSet<typename std::decay<RA>::type>::value, "");
    static_assert(!IsVector<typename std::decay<RA>::type>::value, "");
    auto physical = ra.ToPhysical();
    auto rng = physical.ToRange();
    MergeImpl(ranges::begin(rng), ranges::end(rng));
  }

  void Merge(const std::set<std::tuple<std::string>>& ts) {
    MergeImpl(ts.begin(), ts.end());
  }

  template <typename RA>
  void DeferredMerge(const RA& ra) {
    static_assert(!IsSet<typename std::decay<RA>::type>::value, "");
    static_assert(!IsVector<typename std::decay<RA>::type>::value, "");
    ra::StreamRaInto(ra, &deferred_merge_);
  }

  void DeferredMerge(const std::set<std::tuple<std::string>>& ts) {
    deferred_merge_.insert(ts.begin(), ts.end());
  }

  void Tick() {
    for (const std::tuple<std::string>& t : deferred_merge_) {
      std::cout << std::get<0>(t) << std::endl;
    }
    deferred_merge_.clear();
  }

 private:
  template <typename Iterator, typename Sentinel>
  void MergeImpl(Iterator iter, Sentinel end) {
    for (; iter != end; ++iter) {
      using tuple_type = typename std::decay<decltype(*iter)>::type;
      using value_type =
          typename std::decay<decltype(std::get<0>(*iter))>::type;
      static_assert(std::tuple_size<tuple_type>::value == 1,
                    "Stdout can only merge string tuples..");
      static_assert(std::is_same<value_type, std::string>::value,
                    "Stdout can only merge strings.");
      std::cout << std::get<0>(*iter) << std::endl;
    }
  }

  std::set<std::tuple<std::string>> deferred_merge_;
};

}  // namespace fluent

#endif  // FLUENT_STDOUT_H_
