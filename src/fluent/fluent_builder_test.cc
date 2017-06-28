#include "fluent/fluent_builder.h"

#include <cstddef>

#include <chrono>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "common/hash_util.h"
#include "common/mock_pickler.h"
#include "common/status.h"
#include "fluent/infix.h"
#include "fluent/timestamp_wrapper.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/mock_to_sql.h"
#include "lineagedb/noop_client.h"
#include "ra/logical/all.h"
#include "testing/mock_clock.h"

namespace fluent {

namespace ldb = lineagedb;

TEST(FluentBuilder, SimpleBuildCheckNoLogicalTimestamp) {
  zmq::context_t context(1);
  ldb::ConnectionConfig conf;
  auto fb_or = fluent<ldb::NoopClient>("n", "inproc://a", &context, conf);
  ASSERT_EQ(Status::OK, fb_or.status());
  auto fe_or = fb_or.ConsumeValueOrDie()
                   .table<std::string, int>("t", {{"x", "y"}})
                   .scratch<std::string, int>("s", {{"x", "y"}})
                   .channel<std::string, int>("c", {{"addr", "x"}})
                   .stdin()
                   .stdout()
                   .periodic("p", std::chrono::milliseconds(100))
                   .RegisterBootstrapRules([](auto& t, auto& s, auto& c,
                                              auto& in, auto& out, auto& p) {
                     UNUSED(t);
                     UNUSED(s);
                     UNUSED(c);
                     UNUSED(in);
                     UNUSED(out);
                     UNUSED(p);
                     return std::make_tuple();
                   })
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

TEST(FluentBuilder, SimpleBuildCheckWithLogicalTimestamp) {
  zmq::context_t context(1);
  ldb::ConnectionConfig config;
  auto fb_or =
      fluent<ldb::NoopClient, Hash, ldb::MockToSql, MockPickler, MockClock>(
          "name", "inproc://addr", &context, config);
  ASSERT_EQ(Status::OK, fb_or.status());
  auto fe_or =
      fb_or.ConsumeValueOrDie()
          .logical_time()
          .table<std::string, int>("t", {{"x", "y"}})
          .scratch<std::string, int>("s", {{"x", "y"}})
          .channel<std::string, int>("c", {{"addr", "x"}})
          .stdin()
          .stdout()
          .periodic("p", std::chrono::milliseconds(100))
          .RegisterBootstrapRules([](TimestampWrapper& logical_time, auto& t,
                                     auto& s, auto& c, auto& in, auto& out,
                                     auto& p) {
            UNUSED(logical_time);
            UNUSED(t);
            UNUSED(s);
            UNUSED(c);
            UNUSED(in);
            UNUSED(out);
            UNUSED(p);
            return std::make_tuple();
          })
          .RegisterRules([](TimestampWrapper& logical_time, auto& t, auto& s,
                            auto& c, auto& in, auto& out, auto& p) {
            UNUSED(logical_time);
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
            return std::make_tuple(rule0, rule1, rule2, rule3, rule4, rule5,
                                   rule6);
          });
  EXPECT_EQ(Status::OK, fe_or.status());
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
