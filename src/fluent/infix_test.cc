#include "fluent/infix.h"

#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "collections/table.h"
#include "common/macros.h"
#include "common/mock_pickler.h"
#include "fluent/rule.h"
#include "fluent/rule_tags.h"
#include "ra/logical/all.h"

namespace lra = fluent::ra::logical;

namespace fluent {

TEST(Infix, Table) {
  Table<int> t("t", {{"x"}});
  std::set<std::tuple<int>> xs;
  auto ra = lra::make_iterable(&xs);

  using namespace fluent::infix;
  Rule<Table<int>, MergeTag, decltype(ra)> merge = t <= ra;
  Rule<Table<int>, DeferredMergeTag, decltype(ra)> deferred_merge = t += ra;
  Rule<Table<int>, DeferredDeleteTag, decltype(ra)> deferred_delete = t -= ra;
  UNUSED(merge);
  UNUSED(deferred_merge);
  UNUSED(deferred_delete);
}

TEST(Infix, Channel) {
  Channel<MockPickler, std::string> c(42, "c", {{"addr"}}, nullptr);
  std::set<std::tuple<std::string>> xs;
  auto ra = lra::make_iterable(&xs);

  using namespace fluent::infix;
  Rule<Channel<MockPickler, std::string>, MergeTag, decltype(ra)> merge =
      c <= ra;
  UNUSED(merge);
}

TEST(Infix, Scratch) {
  Scratch<int> s("s", {{"x"}});
  std::set<std::tuple<int>> xs;
  auto ra = lra::make_iterable(&xs);

  using namespace fluent::infix;
  Rule<Scratch<int>, MergeTag, decltype(ra)> merge = s <= ra;
  UNUSED(merge);
}

TEST(Infix, Stdout) {
  Stdout stdout_;
  std::set<std::tuple<std::string>> xs;
  auto ra = lra::make_iterable(&xs);

  using namespace fluent::infix;
  Rule<Stdout, MergeTag, decltype(ra)> merge = stdout_ <= ra;
  Rule<Stdout, DeferredMergeTag, decltype(ra)> deferred_merge = stdout_ += ra;
  UNUSED(merge);
  UNUSED(deferred_merge);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
