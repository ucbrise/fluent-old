#ifndef FLUENT_RULE_TAGS_H_
#define FLUENT_RULE_TAGS_H_

namespace fluent {

// Bloom [1] had four types of rules:
//
//   1. Merge (e.g. t <= u)
//   2. Deferred merge (e.g. t <+ u)
//   3. Deferred delete (e.g. t <- u)
//   4. Asynchronous merge (e.g. t <~ u)
//
// In fluent, we adopt the first three types of rules with some minor
// modifications:
//
//   - For channels, we let merge behave like asynchronous merge.
//   - For scratches, we do not support deferred merge or deferred delete. The
//     scratch will be cleared at the end of the tick anyway, so adding or
//     removing from it is wasted work.
//
// To summarize, here are the rules that each collection supports.
//
//   |         | merge | deferred merge | deferred delete |
//   | ------- | ----- | -------------- | --------------- |
//   | table   | y     | y              | y               |
//   | scratch | y     | n              | n               |
//   | channel | y     | n              | n               |
//
// A FluentExecutor is constructed with a tuple of rules, where each rule is a
// tuple (lhs, type, rhs) where
//
//   - lhs is a pointer to a collection,
//   - type is an instance of one of the structs below, and
//   - rhs is a relational algebra expression.
//
// The fluent executor dispatches on the type of `type` to determine how to
// evaluate the rule. More specifically,
//
//   - If type is a MergeTag, then lhs->Merge(rhs) is called.
//   - If type is a DeferredMergeTag, then lhs->DeferredMerge(rhs) is called.
//   - If type is a DeferredDeleteTag, then lhs->DeferredDelete(rhs) is called.
//
// Note that these tags act very similarly to iterator tags [2] from the
// standard library.
//
// [1]: http://db.cs.berkeley.edu/papers/cidr11-bloom.pdf
// [2]: http://en.cppreference.com/w/cpp/iterator/iterator_tags
struct MergeTag {};
struct DeferredMergeTag {};
struct DeferredDeleteTag {};

}  // namespace fluent

#endif  // FLUENT_RULE_TAGS_H_
