#include "fluent/rule.h"

#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "fluent/rule_tags.h"
#include "fluent/table.h"
#include "ra/all.h"

namespace fluent {

TEST(Rule, ToDebugString) {
  Table<int> t("t", {{"x"}});
  std::set<std::tuple<int>> xs;
  const Rule<Table<int>*, MergeTag, ra::Iterable<std::set<std::tuple<int>>>>
      rule{&t, MergeTag(), ra::make_iterable("xs", &xs)};
  EXPECT_EQ(rule.ToDebugString(), "t <= xs");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
