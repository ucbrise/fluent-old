#ifndef FLUENT_RULE_TAGS_H_
#define FLUENT_RULE_TAGS_H_

#include <string>

namespace fluent {

// Bloom [1] had four types of rules:
//
//   1. Merge (e.g. t <= u)
//   2. Deferred merge (e.g. t <+ u)
//   3. Deferred delete (e.g. t <- u)
//   4. Asynchronous merge (e.g. t <~ u)
//
// Fluent has the same rules, except that asynchronous merge is collapsed into
// merge. The following table describes what rules each collection supports:
//
//   |         | merge | deferred merge | deferred delete |
//   | ------- | ----- | -------------- | --------------- |
//   | table   | y     | y              | y               |
//   | scratch | y     | n              | n               |
//   | channel | y     | n              | n               |
//   | stdin   | n     | n              | n               |
//   | stdout  | y     | y              | n               |
//
// In Bloom, rules look something like this:
//
//   foo <= bar.cross(baz)
//
// where
//
//   - `foo` is the *head* of the rule,
//   - `<=` describes the type of the rule, and
//   - `bar.cross(baz)` is the *body* of the rule.
//
// Fluent represents rules in exactly the same way. More concretely, each rule
// is a tuple (lhs, type, rhs) where:
//
//   - `lhs` is a pointer to a collection,
//   - `type` is an instance of one of the structs below, and
//   - `rhs` is a relational algebra expression.
//
// A FluentExecutor takes a list (techincally, a tuple) of rules when it is
// constructed. The FluentExecutor then dispatches on the type of `type` to
// determine how to evaluate the rule. More specifically,
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
struct RuleTag {
  virtual ~RuleTag() {}
};

struct MergeTag : public RuleTag {
  std::string ToDebugString() const { return "<="; }
};

struct DeferredMergeTag : public RuleTag {
  std::string ToDebugString() const { return "+="; }
};

struct DeferredDeleteTag : public RuleTag {
  std::string ToDebugString() const { return "-="; }
};

}  // namespace fluent

#endif  // FLUENT_RULE_TAGS_H_
