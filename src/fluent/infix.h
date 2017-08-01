#ifndef FLUENT_INFIX_H_
#define FLUENT_INFIX_H_

#include "collections/channel.h"
#include "collections/scratch.h"
#include "collections/stdout.h"
#include "collections/table.h"
#include "fluent/rule.h"
#include "fluent/rule_tags.h"

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
//   Table<int> t("t", {{"x"}});
//   Scratch<float> s("s", {{"y"}});
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
template <typename... Ts, typename LogicalRa>
Rule<collections::Table<Ts...>, MergeTag, typename std::decay<LogicalRa>::type>
operator<=(collections::Table<Ts...>& t, LogicalRa&& rhs) {
  return {&t, MergeTag(), std::forward<LogicalRa>(rhs)};
}

// Table +=
template <typename... Ts, typename LogicalRa>
Rule<collections::Table<Ts...>, DeferredMergeTag,
     typename std::decay<LogicalRa>::type>
operator+=(collections::Table<Ts...>& t, LogicalRa&& rhs) {
  return {&t, DeferredMergeTag(), std::forward<LogicalRa>(rhs)};
}

// Table -=
template <typename... Ts, typename LogicalRa>
Rule<collections::Table<Ts...>, DeferredDeleteTag,
     typename std::decay<LogicalRa>::type>
operator-=(collections::Table<Ts...>& t, LogicalRa&& rhs) {
  return {&t, DeferredDeleteTag(), std::forward<LogicalRa>(rhs)};
}

// Channel <=
template <template <typename> class Pickler, typename T, typename... Ts,
          typename LogicalRa>
Rule<collections::Channel<Pickler, T, Ts...>, MergeTag,
     typename std::decay<LogicalRa>::type>
operator<=(collections::Channel<Pickler, T, Ts...>& c, LogicalRa&& rhs) {
  return {&c, MergeTag(), std::forward<LogicalRa>(rhs)};
}

// Scratch <=
template <typename... Ts, typename LogicalRa>
Rule<collections::Scratch<Ts...>, MergeTag,
     typename std::decay<LogicalRa>::type>
operator<=(collections::Scratch<Ts...>& s, LogicalRa&& rhs) {
  return {&s, MergeTag(), std::forward<LogicalRa>(rhs)};
}

// Stdout <=
template <typename... Ts, typename LogicalRa>
Rule<collections::Stdout, MergeTag, typename std::decay<LogicalRa>::type>
operator<=(collections::Stdout& o, LogicalRa&& rhs) {
  return {&o, MergeTag(), std::forward<LogicalRa>(rhs)};
}

// Stdout +=
template <typename LogicalRa>
Rule<collections::Stdout, DeferredMergeTag,
     typename std::decay<LogicalRa>::type>
operator+=(collections::Stdout& o, LogicalRa&& rhs) {
  return {&o, DeferredMergeTag(), std::forward<LogicalRa>(rhs)};
}

}  // namespace infix
}  // namespace fluent

#endif  // FLUENT_INFIX_H_
