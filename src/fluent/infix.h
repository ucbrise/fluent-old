#ifndef FLUENT_INFIX_H_
#define FLUENT_INFIX_H_

#include <tuple>

#include "fluent/rule_tags.h"
#include "fluent/table.h"

namespace fluent {
namespace infix {

// # Overview
// Recall from `rule_tags.h` (which you should read, if you haven't already)
// that fluent represents a rule as a triple (lhs, type, rhs) where:
//
//   - `lhs` is a pointer to a collection,
//   - `type` is an instance of one of the structs below, and
//   - `rhs` is a relational algebra expression.
//
// This file provides three infix functions:
//
//   - <= (merge)
//   - += (deferred merge)
//   - -= (deferred delete)
//
// to make constructing rules quite a bit sweeter. For example, consider the
// following snippet of code.
//
//   using namespace fluent::infix;
//   Table<int> t;
//   Scratch<float> s;
//   auto rule = t <= s.Iterable() | ra::count();
//
// Here, `rule` is the tuple `(&t, MergeTag(), s.Iterable() | ra::count())`.
// See `fluent_executor_test.cc` for more complete examples of how to construct
// rules and pass them to a FluentExecutor.
//
// # Some Things to Note
// - Every infix function takes its first argument as a non-const reference.
//   Yes, this is disallowed by the Google style guide, but since these
//   functions exist purely for syntactic sugar, I think it's okay.
// - The rules returned by calling these infix functions return a pointer to
//   their first argument. Thus, you must make sure that the first argument
//   outlives the rule.
// - The infix functions live in their own namespace `fluent::infix` for code
//   hygiene (similar to std::literals::string_literals or
//   std::literals::chrono_literals), so make sure to add `using namespace
//   fluent::infix` before using the functions.

// Table <=
template <typename... Ts, typename Rhs>
std::tuple<Table<Ts...>*, MergeTag, typename std::decay<Rhs>::type> operator<=(
    Table<Ts...>& t, Rhs&& rhs) {
  return {&t, MergeTag(), std::forward<Rhs>(rhs)};
}

// Table +=
template <typename... Ts, typename Rhs>
std::tuple<Table<Ts...>*, DeferredMergeTag, typename std::decay<Rhs>::type>
operator+=(Table<Ts...>& t, Rhs&& rhs) {
  return {&t, DeferredMergeTag(), std::forward<Rhs>(rhs)};
}

// Table -=
template <typename... Ts, typename Rhs>
std::tuple<Table<Ts...>*, DeferredDeleteTag, typename std::decay<Rhs>::type>
operator-=(Table<Ts...>& t, Rhs&& rhs) {
  return {&t, DeferredDeleteTag(), std::forward<Rhs>(rhs)};
}

// Channel <=
template <typename... Ts, typename Rhs>
std::tuple<Channel<Ts...>*, MergeTag, typename std::decay<Rhs>::type>
operator<=(Channel<Ts...>& c, Rhs&& rhs) {
  return {&c, MergeTag(), std::forward<Rhs>(rhs)};
}

// Scratch <=
template <typename... Ts, typename Rhs>
std::tuple<Scratch<Ts...>*, MergeTag, typename std::decay<Rhs>::type>
operator<=(Scratch<Ts...>& s, Rhs&& rhs) {
  return {&s, MergeTag(), std::forward<Rhs>(rhs)};
}

// Stdout <=
template <typename... Ts, typename Rhs>
std::tuple<Stdout*, MergeTag, typename std::decay<Rhs>::type> operator<=(
    Stdout& o, Rhs&& rhs) {
  return {&o, MergeTag(), std::forward<Rhs>(rhs)};
}

// Stdout +=
template <typename Rhs>
std::tuple<Stdout*, DeferredMergeTag, typename std::decay<Rhs>::type>
operator+=(Stdout& o, Rhs&& rhs) {
  return {&o, DeferredMergeTag(), std::forward<Rhs>(rhs)};
}

// Lattices <=
template <typename T, typename Rhs>
std::tuple<Lattice<T>*, MergeTag, typename std::decay<Rhs>::type> operator<=(
    Lattice<T>& l, Rhs&& rhs) {
  return {&l, MergeTag(), std::forward<Rhs>(rhs)};
}

// CompositeLattices <=
template <typename K, typename V, typename Rhs>
std::tuple<MapLattice<K, V>*, MergeTag, typename std::decay<Rhs>::type> operator<=(
    MapLattice<K, V>& l, Rhs&& rhs) {
  return {&l, MergeTag(), std::forward<Rhs>(rhs)};
}

}  // namespace infix
}  // namespace fluent

#endif  // FLUENT_INFIX_H_
