#ifndef FLUENT_RULE_H_
#define FLUENT_RULE_H_

#include "fmt/format.h"

#include "collections/collection.h"
#include "collections/collection_util.h"
#include "common/static_assert.h"
#include "ra/logical/to_debug_string.h"

namespace fluent {

template <typename Collection_, typename RuleTag, typename LogicalRa>
struct Rule {
  using is_collection = std::is_base_of<Collection, Collection_>;
  static_assert(StaticAssert<is_collection>::value, "");

  using collection_types = typename CollectionTypes<Collection_>::type;
  using logical_ra_types = typename LogicalRa::column_types;
  using equal_types = std::is_same<collection_types, logical_ra_types>;
  static_assert(StaticAssert<equal_types>::value, "");

  Collection_* collection;
  RuleTag rule_tag;
  LogicalRa ra;

  std::string ToDebugString() const {
    return fmt::format("{} {} {}", collection->Name(), rule_tag.ToDebugString(),
                       fluent::ra::logical::ToDebugString(ra));
  }
};

}  // namespace fluent

#endif  // FLUENT_RULE_H_
