#include "fluent/fluent_builder.h"

#include <cstddef>

#include <chrono>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "common/status.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/noop_client.h"
#include "ra/logical/all.h"

namespace fluent {

TEST(FluentBuilder, SimpleBuildCheck) {
  zmq::context_t context(1);
  lineagedb::ConnectionConfig conf;
  auto fb_or = fluent<lineagedb::NoopClient>("n", "inproc://a", &context, conf);
  ASSERT_EQ(Status::OK, fb_or.status());
  auto fe_or = fb_or.ConsumeValueOrDie()
                   .table<std::string, int>("t", {{"x", "y"}})
                   .scratch<std::string, int>("s", {{"x", "y"}})
                   .channel<std::string, int>("c", {{"addr", "x"}})
                   .stdin()
                   .stdout()
                   .periodic("p", std::chrono::milliseconds(100))
                   .RegisterRules([](auto& t, auto& s, auto& c, auto& in,
                                     auto& out, auto& p) {
                     UNUSED(in);
                     UNUSED(p);
                     using namespace fluent::infix;
                     using namespace fluent::ra::logical;
                     auto rule0 = t <= make_collection(&s);
                     auto rule1 = t += make_collection(&s);
                     auto rule2 = t -= make_collection(&s);
                     auto rule3 = s <= make_collection(&s);
                     auto rule4 = c <= make_collection(&s);
                     auto rule5 = out <= (make_collection(&s) | project<0>());
                     auto rule6 = out += (make_collection(&s) | project<0>());
                     return std::make_tuple(rule0, rule1, rule2, rule3, rule4,
                                            rule5, rule6);
                   });
  EXPECT_EQ(Status::OK, fe_or.status());
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
