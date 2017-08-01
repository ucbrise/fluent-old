#include "fluent/rule.h"

#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "collections/table.h"
#include "fluent/rule_tags.h"
#include "ra/logical/all.h"

namespace fluent {

namespace lra = ra::logical;

using collections::Table;

TEST(Rule, ToDebugString) {
  Table<int> t("t", {{"x"}});
  std::set<std::tuple<int>> xs;
  auto ra = lra::make_iterable(&xs);
  Rule<Table<int>, MergeTag, decltype(ra)> rule{&t, MergeTag(), ra};
  EXPECT_EQ(rule.ToDebugString(), "t <= Iterable");
}

// This code should NOT compile.
TEST(Rule, MismatchedTypes) {
  // Table<int> t("t", {{"x"}});
  // std::set<std::tuple<int, int>> xs;
  // auto ra = lra::make_iterable(&xs);
  // Rule<Table<int>, MergeTag, decltype(ra)> rule{&t, MergeTag(), ra};
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
