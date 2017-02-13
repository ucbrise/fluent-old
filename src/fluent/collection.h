#ifndef FLUENT_COLLECTION_H_
#define FLUENT_COLLECTION_H_

#include <functional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "range/v3/all.hpp"

#include "fluent/serialization.h"
#include "ra/iterable.h"

namespace fluent {

// A Collection is a Table, a Scratch, or a Channel (see table.h, scratch.h,
// and channel.h). Each Collection is essentially a relation with slightly
// different behavior. For more information, refer to the Bloom paper from
// which these terms were taken [1].
//
// [1]: http://db.cs.berkeley.edu/papers/cidr11-bloom.pdf
template <typename... Ts>
class Collection {
 public:
  explicit Collection(const std::string& name) : name_(name) {}

  const std::string& Name() const { return name_; }

  const std::set<std::tuple<Ts...>>& Get() const { return ts_; }

  ra::Iterable<std::set<std::tuple<Ts...>>> Iterable() const {
    return ra::make_iterable(&ts_);
  }

  // Tick should be invoked after each iteration of computation. The behavior
  // depends on the collection type.
  //
  // - Table:   no-op.
  // - Scratch: clears the collection.
  // - Channel: clears the collection.
  virtual void Tick() = 0;

 protected:
  std::set<std::tuple<Ts...>>& MutableGet() { return ts_; }

 private:
  const std::string name_;

  // TODO(mwhittaker): Right now, as a short-term simplification to get the
  // ball rolling, we store tuples as sets. We should have Collections have
  // keys. Doing this will require us to store a map from key tuples to body
  // tuples. It will also force us to do a tiny bit of metaprogramming to allow
  // users to write something like `Collection<Key<int, float>, Body<char,
  // int>>`.
  std::set<std::tuple<Ts...>> ts_;
};

}  // namespace fluent

#endif  // FLUENT_COLLECTION_H_
