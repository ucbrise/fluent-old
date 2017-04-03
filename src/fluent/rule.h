#ifndef FLUENT_RULE_H_
#define FLUENT_RULE_H_

#include "fmt/format.h"

namespace fluent {

template <typename CollectionPointer, typename RuleTag, typename RA>
struct Rule {
  CollectionPointer collection;
  RuleTag rule_tag;
  RA ra;

  std::string ToDebugString() const {
    return fmt::format("{} {} {}", collection->Name(), rule_tag.ToDebugString(),
                       ra.ToDebugString());
  }
};

}  // namespace fluent

#endif  // FLUENT_RULE_H_
