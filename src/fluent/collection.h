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

namespace detail {

// See `GetParser`.
template <typename... Ts, std::size_t... Is>
std::tuple<Ts...> parse_tuple_impl(const std::vector<std::string>& columns,
                                   std::index_sequence<Is...>) {
  return {FromString<Ts>(columns[Is])...};
}

// See `GetParser`.
template <typename... Ts>
std::tuple<Ts...> parse_tuple(const std::vector<std::string>& columns) {
  using Indices = std::make_index_sequence<sizeof...(Ts)>;
  return parse_tuple_impl<Ts...>(columns, Indices());
}

}  // namespace detail

// A Collection is a Table, a Scratch, or a Channel (see table.h, scratch.h,
// and channel.h). Each Collection is essentially a relation with slightly
// different behavior. For more information, refer to the Bloom paper from
// which these terms were taken [1].
//
// [1]: http://db.cs.berkeley.edu/papers/cidr11-bloom.pdf
//
// TODO(mwhittaker): Implement other basic methods like delete, deferred add,
// deferred delete, etc..
// TODO(mwhittaker): Eventually, we'll want to perform more complex operations
// over the collections (e.g. relation operators). Figure out a nice way to tie
// our existing work on relational operators into this code. We might, for
// example, have a method to return a range which would allow users to chain
// together complicated relational operators. Then, we could have a method that
// takes a range and merges it into the table (making sure not to invalidate
// any iterators).
// TODO(mwhittaker): We never use anything of type `Collection*` or
// `Collection&`. The driver does sufficient metaprogramming to deal explicitly
// with Tables, Scrathes, and Collections. So, we could remove the Collection
// type and replace it with a concept.
template <typename... Ts>
class Collection {
 public:
  // All Collections across all communicating programs must be uniquely named.
  //
  // TODO(mwhittaker): Think about if it's possible to enforce this in any sane
  // way.
  explicit Collection(const std::string& name) : name_(name) {}

  const std::string& Name() const { return name_; }

  const std::set<std::tuple<Ts...>>& Get() const { return ts_; }

  ra::Iterable<std::set<std::tuple<Ts...>>> Iterable() {
    return ra::make_iterable(&ts_);
  }

  // - Table:   merge (<=) operation.
  // - Scratch: merge (<=) operation.
  // - Channel: asynchronous merge (<~) operation.
  virtual void Add(const std::tuple<Ts...>& t) = 0;

  // `AddRelalg(q)` executes `q` and calls `Add(t)` on every tuple `t` in the
  // result.
  template <typename T>
  void AddRelalg(T query) {
    // If query includes an iterable over ts_, then inserting into ts_ might
    // invalidate the iterator. Thus, we first write into a temprorary vector
    // and then into the ts_.
    // TODO(mwhittaker): Figure out if this is necessary.
    std::vector<std::tuple<Ts...>> buf;
    auto physical = query.ToPhysical();
    auto rng = physical.ToRange();
    for (auto iter = ranges::begin(rng); iter != ranges::end(rng); ++iter) {
      buf.push_back(*iter);
    }
    for (std::tuple<Ts...> t : buf) {
      ts_.insert(std::move(t));
    }
  }

  // Tick should be invoked after each iteration of computation. The behavior
  // depends on the collection type.
  //
  // - Table:   no-op.
  // - Scratch: clears the collection.
  // - Channel: clears the collection.
  virtual void Tick() = 0;

  // `Collection<T1, ..., Tn>.GetParser()(columns)` parses a vector of `n`
  // strings into a tuple of type `std::tuple<T1, ..., Tn>` and inserts it into
  // the collection. The ith element of the tuple is parsed using a global `Ti
  // FromString<Ti>(const std::string&)` function which is assumed to exists
  // (see `serialization.h` for more information).
  std::function<void(const std::vector<std::string>& columns)> GetParser() {
    return [this](const std::vector<std::string>& columns) {
      this->ts_.insert(detail::parse_tuple<Ts...>(columns));
    };
  }

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
