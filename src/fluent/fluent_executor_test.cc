#include "fluent/fluent_executor.h"

#include <cstddef>
#include <cstdint>

#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "collections/channel.h"
#include "collections/collection_tuple_ids.h"
#include "common/mock_pickler.h"
#include "common/status.h"
#include "common/status_or.h"
#include "common/string_util.h"
#include "fluent/fluent_builder.h"
#include "fluent/infix.h"
#include "fluent/local_tuple_id.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/mock_client.h"
#include "lineagedb/mock_to_sql.h"
#include "lineagedb/noop_client.h"
#include "ra/logical/all.h"
#include "testing/captured_stdout.h"
#include "testing/mock_clock.h"

namespace ldb = fluent::lineagedb;
namespace lra = fluent::ra::logical;

using ::testing::UnorderedElementsAreArray;

namespace fluent {

auto noopfluent(const std::string& name, const std::string& address,
                zmq::context_t* context,
                const ldb::ConnectionConfig& connection_config) {
  return fluent<ldb::NoopClient, Hash, ldb::ToSql, MockPickler, MockClock>(
      name, address, context, connection_config);
}

TEST(FluentExecutor, SimpleProgram) {
  zmq::context_t context(1);
  ldb::ConnectionConfig connection_config;
  std::set<std::tuple<int>> xs = {{0}};
  std::map<std::tuple<int>, CollectionTupleIds> expected;
  Hash<std::tuple<int>> hash;

  auto fb_or = noopfluent("name", "inproc://yolo", &context, connection_config);
  ASSERT_EQ(Status::OK, fb_or.status());
  auto fe_or = fb_or.ConsumeValueOrDie()
                   .table<int>("t", {{"x"}})
                   .scratch<int>("s", {{"x"}})
                   .RegisterRules([&xs](auto& t, auto& s) {
                     using namespace fluent::infix;
                     auto rule1 = t <= lra::make_iterable(&xs);
                     auto rule2 = s <= lra::make_collection(&t);
                     auto rule3 =
                         t <= (lra::make_collection(&s) |
                               lra::map([](const std::tuple<int>& t) {
                                 return std::make_tuple(std::get<0>(t) + 1);
                               }));
                     return std::make_tuple(rule1, rule2, rule3);
                   });
  ASSERT_EQ(Status::OK, fe_or.status());
  auto f = fe_or.ConsumeValueOrDie();

  // | time | action | delta t | delta s |
  // | ---- | ------ | ------- | ------- |
  // | 1    | rule 1 | +0      |         |
  // | 2    | rule 2 |         | +0      |
  // | 3    | rule 3 | +1      |         |
  // | 4    | tick   |         | -0      |
  // | 5    | rule 1 | +0      |         |
  // | 6    | rule 2 |         | +0,1    |
  // | 7    | rule 3 | +1,2    |         |
  // | 8    | tick   |         | -0,1    |
  // | 9    | rule 1 | +0      |         |
  // | 10   | rule 2 |         | +0,1,2  |
  // | 11   | rule 3 | +1,2,3  |         |
  // | 12   | tick   |         | -0,1,2  |

  expected = {};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  EXPECT_EQ(f.Get<1>().Get(), expected);

  ASSERT_EQ(Status::OK, f.Tick());
  expected = {{{0}, {hash({0}), {1}}}, {{1}, {hash({1}), {3}}}};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  expected = {};
  EXPECT_EQ(f.Get<1>().Get(), expected);

  ASSERT_EQ(Status::OK, f.Tick());
  expected = {{{0}, {hash({0}), {1, 5}}},
              {{1}, {hash({1}), {3, 7}}},
              {{2}, {hash({2}), {7}}}};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  expected = {};
  EXPECT_EQ(f.Get<1>().Get(), expected);

  ASSERT_EQ(Status::OK, f.Tick());
  expected = {{{0}, {hash({0}), {1, 5, 9}}},
              {{1}, {hash({1}), {3, 7, 11}}},
              {{2}, {hash({2}), {7, 11}}},
              {{3}, {hash({3}), {11}}}};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  expected = {};
  EXPECT_EQ(f.Get<1>().Get(), expected);
}

TEST(FluentExecutor, SimpleBootstrap) {
  zmq::context_t context(1);
  lineagedb::ConnectionConfig connection_config;
  std::set<std::tuple<int>> xs = {{1}, {2}, {3}};
  std::map<std::tuple<int>, CollectionTupleIds> expected;
  Hash<std::tuple<int>> hash;

  auto fb_or = noopfluent("name", "inproc://yolo", &context, connection_config);
  ASSERT_EQ(Status::OK, fb_or.status());
  auto fe_or =
      fb_or.ConsumeValueOrDie()
          .table<int>("t", {{"x"}})
          .scratch<int>("s", {{"x"}})
          .RegisterBootstrapRules([&xs](auto& t, auto& s) {
            using namespace fluent::infix;
            auto rule1 = t <= lra::make_iterable(&xs);
            auto rule2 = s <= lra::make_iterable(&xs);
            return std::make_tuple(rule1, rule2);
          })
          .RegisterRules([&xs](auto&, auto&) { return std::make_tuple(); });
  ASSERT_EQ(Status::OK, fe_or.status());
  auto f = fe_or.ConsumeValueOrDie();

  expected = {};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  EXPECT_EQ(f.Get<1>().Get(), expected);

  ASSERT_EQ(Status::OK, f.BootstrapTick());
  expected = {{{1}, {hash({1}), {1}}},
              {{2}, {hash({2}), {1}}},
              {{3}, {hash({3}), {1}}}};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  expected = {};
  EXPECT_EQ(f.Get<1>().Get(), expected);
}

TEST(FluentExecutor, ComplexProgram) {
  auto add1_mult2 = [](const std::tuple<int>& t) {
    return std::tuple<int>((1 + std::get<0>(t)) * 2);
  };
  auto is_even = [](const std::tuple<int>& t) {
    return std::get<0>(t) % 2 == 0;
  };
  auto to_string = [](const std::tuple<int>& t) {
    return std::tuple<std::string>(std::to_string(std::get<0>(t)));
  };

  zmq::context_t context(1);
  lineagedb::ConnectionConfig connection_config;
  std::set<std::tuple<int>> xs = {{0}};
  std::map<std::tuple<int>, CollectionTupleIds> expected;
  Hash<std::tuple<int>> hash;
  CapturedStdout captured;

  auto fb_or = noopfluent("name", "inproc://yolo", &context, connection_config);
  ASSERT_EQ(Status::OK, fb_or.status());
  auto fe_or =
      fb_or.ConsumeValueOrDie()
          .table<int>("t", {{"x"}})
          .scratch<int>("s", {{"x"}})
          .stdout()
          .RegisterBootstrapRules([&](auto& t, auto&, auto&) {
            using namespace fluent::infix;
            auto a = t <= lra::make_iterable(&xs);
            return std::make_tuple(a);
          })
          .RegisterRules([&](auto& t, auto& s, auto& out) {
            using namespace fluent::infix;
            auto b = t <= (lra::make_collection(&t) | lra::map(add1_mult2));
            auto c = s <= lra::make_collection(&t);
            auto d = t -= (lra::make_collection(&s) | lra::filter(is_even));
            auto e = out <= (lra::make_collection(&t) | lra::map(to_string));
            return std::make_tuple(b, c, d, e);
          });
  ASSERT_EQ(Status::OK, fe_or.status());
  auto f = fe_or.ConsumeValueOrDie();

  // | time | action | delta t | delta s | delta out |
  // | ---- | ------ | ------- | ------- | --------- |
  // | 1    | rule a | +0      |         |           |
  // | 2    | tick   |         |         |           |
  // | 3    | rule b | +2      |         |           |
  // | 4    | rule c |         | +0, 2   |           |
  // | 5    | rule d |         |         |           |
  // | 6    | rule e |         |         | +0,2      |
  // | 7    | tick   | -0,2    |         | +0,2      |

  expected = {};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  EXPECT_EQ(f.Get<1>().Get(), expected);
  EXPECT_STREQ("", captured.Get().c_str());

  ASSERT_EQ(Status::OK, f.BootstrapTick());
  expected = {{{0}, {hash({0}), {1}}}};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  expected = {};
  EXPECT_EQ(f.Get<1>().Get(), expected);
  EXPECT_STREQ("", captured.Get().c_str());

  ASSERT_EQ(Status::OK, f.Tick());
  expected = {};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  expected = {};
  EXPECT_EQ(f.Get<1>().Get(), expected);
  EXPECT_STREQ("0\n2\n", captured.Get().c_str());

  ASSERT_EQ(Status::OK, f.Tick());
  expected = {};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  expected = {};
  EXPECT_EQ(f.Get<1>().Get(), expected);
  EXPECT_STREQ("0\n2\n", captured.Get().c_str());
}

TEST(FluentExecutor, SimpleCommunication) {
  auto reroute = [](const std::string& s) {
    return [s](const std::tuple<std::string, int>& t) {
      return std::make_tuple(s, std::get<1>(t));
    };
  };

  zmq::context_t context(1);
  lineagedb::ConnectionConfig conn_config;
  std::set<std::tuple<std::string, int>> xs = {{"inproc://pong", 42}};
  std::map<std::tuple<std::string, int>, CollectionTupleIds> expected;
  Hash<std::tuple<std::string, int>> hash;

  auto ping_fb_or = noopfluent("name", "inproc://ping", &context, conn_config);
  ASSERT_EQ(Status::OK, ping_fb_or.status());
  auto ping_fe_or = ping_fb_or.ConsumeValueOrDie()
                        .channel<std::string, int>("c", {{"addr", "x"}})
                        .RegisterBootstrapRules([&xs](auto& c) {
                          using namespace fluent::infix;
                          auto brule = c <= lra::make_iterable(&xs);
                          return std::make_tuple(brule);
                        })
                        .RegisterRules([&reroute](auto& c) {
                          using namespace fluent::infix;
                          auto rule = c <= (lra::make_collection(&c) |
                                            lra::map(reroute("inproc://pong")));
                          return std::make_tuple(rule);
                        });
  ASSERT_EQ(Status::OK, ping_fe_or.status());
  auto ping = ping_fe_or.ConsumeValueOrDie();

  auto pong_fb_or = noopfluent("name", "inproc://pong", &context, conn_config);
  ASSERT_EQ(Status::OK, pong_fb_or.status());
  auto pong_fe_or = pong_fb_or.ConsumeValueOrDie()
                        .channel<std::string, int>("c", {{"addr", "x"}})
                        .RegisterRules([&reroute](auto& c) {
                          using namespace fluent::infix;
                          auto rule = c <= (lra::make_collection(&c) |
                                            lra::map(reroute("inproc://ping")));
                          return std::make_tuple(rule);
                        });
  ASSERT_EQ(Status::OK, pong_fe_or.status());
  auto pong = pong_fe_or.ConsumeValueOrDie();

  // | ping time | action | delta c |
  // | --------- | ------ | ------- |
  // | 1         | brule  | +-pong  |
  // | 2         | tick   |         |
  // | 3         | recv   | +ping   |
  // | 4         | rule   |         |
  // | 5         | tick   | -ping   |
  //
  // | pong time | action | delta c |
  // | --------- | ------ | ------- |
  // | 1         | recv   | +pong   |
  // | 2         | rule   |         |
  // | 3         | tick   | -pong   |
  // | 4         | recv   | +pong   |
  // | 5         | rule   |         |
  // | 6         | tick   | -pong   |

  ASSERT_EQ(Status::OK, ping.BootstrapTick());
  expected = {};
  EXPECT_EQ(ping.Get<0>().Get(), expected);

  ASSERT_EQ(Status::OK, pong.Receive());
  expected = {{{"inproc://pong", 42}, {hash({"inproc://pong", 42}), {1}}}};
  ASSERT_EQ(pong.Get<0>().Get(), expected);
  ASSERT_EQ(Status::OK, pong.Tick());
  expected = {};
  ASSERT_EQ(pong.Get<0>().Get(), expected);

  ASSERT_EQ(Status::OK, ping.Receive());
  expected = {{{"inproc://ping", 42}, {hash({"inproc://ping", 42}), {3}}}};
  ASSERT_EQ(ping.Get<0>().Get(), expected);
  ASSERT_EQ(Status::OK, ping.Tick());
  expected = {};
  ASSERT_EQ(ping.Get<0>().Get(), expected);

  ASSERT_EQ(Status::OK, pong.Receive());
  expected = {{{"inproc://pong", 42}, {hash({"inproc://pong", 42}), {4}}}};
  ASSERT_EQ(pong.Get<0>().Get(), expected);
  ASSERT_EQ(Status::OK, pong.Tick());
  expected = {};
  ASSERT_EQ(pong.Get<0>().Get(), expected);
}

TEST(FluentExecutor, SimpleProgramWithLineage) {
  zmq::context_t context(1);
  lineagedb::ConnectionConfig connection_config;
  std::set<std::tuple<int>> xs = {{0}};
  Hash<std::tuple<int>> hash;
  std::map<std::tuple<int>, CollectionTupleIds> expected;

  auto fb_or =
      fluent<ldb::MockClient, Hash, ldb::MockToSql, MockPickler, MockClock>(
          "name", "inproc://yolo", &context, connection_config);
  ASSERT_EQ(Status::OK, fb_or.status());
  auto fe_or = fb_or.ConsumeValueOrDie()
                   .table<int>("t", {{"x"}})
                   .scratch<int>("s", {{"x"}})
                   .RegisterBootstrapRules([&xs](auto& t, auto&) {
                     using namespace fluent::infix;
                     auto brule = t <= lra::make_iterable(&xs);
                     return std::make_tuple(brule);
                   })
                   .RegisterRules([](auto& t, auto& s) {
                     using namespace fluent::infix;
                     auto rule1 =
                         s <= (lra::make_collection(&t) |
                               lra::map([](const std::tuple<int>& t) {
                                 return std::make_tuple(std::get<0>(t) + 1);
                               }));
                     auto rule2 = t <= lra::make_collection(&s);
                     return std::make_tuple(rule1, rule2);
                   });
  ASSERT_EQ(Status::OK, fe_or.status());
  auto f = fe_or.ConsumeValueOrDie();
  const ldb::MockClient<Hash, ldb::MockToSql, MockClock>& client =
      f.GetLineageDbClient();

  using MockClient = ldb::MockClient<Hash, ldb::MockToSql, MockClock>;
  using AddCollectionTuple = MockClient::AddCollectionTuple;
  using AddRuleTuple = MockClient::AddRuleTuple;
  using InsertTupleTuple = MockClient::InsertTupleTuple;
  using DeleteTupleTuple = MockClient::DeleteTupleTuple;
  using AddDerivedLineageTuple = MockClient::AddDerivedLineageTuple;

  using time_point = std::chrono::time_point<MockClock>;
  using std::chrono::seconds;

  // | time | action | delta t | delta s |
  // | ---- | ------ | ------- | ------- |
  // | 1    | brule  | +0      |         |
  // | 2    | tick   |         |         |
  // | 3    | rule1  |         | +1      |
  // | 4    | rule2  | +1      |         |
  // | 5    | tick   |         | -1      |
  // | 6    | rule1  |         | +1,2    |
  // | 7    | rule2  | +1,2    |         |
  // | 8    | tick   |         | -1,2    |

  ASSERT_EQ(client.GetAddCollection().size(), static_cast<std::size_t>(2));
  EXPECT_EQ(client.GetAddCollection()[0],
            AddCollectionTuple("t", "Table", {"x"}, {"int"}));
  EXPECT_EQ(client.GetAddCollection()[1],
            AddCollectionTuple("s", "Scratch", {"x"}, {"int"}));
  ASSERT_EQ(client.GetAddRule().size(), static_cast<std::size_t>(3));
  EXPECT_EQ(client.GetAddRule()[0],  //
            AddRuleTuple(0, true, "t <= Iterable"));
  EXPECT_EQ(client.GetAddRule()[1],
            AddRuleTuple(0, false, "s <= Map(Collection(t))"));
  EXPECT_EQ(client.GetAddRule()[2],
            AddRuleTuple(1, false, "t <= Collection(s)"));
  EXPECT_EQ(client.GetInsertTuple().size(), static_cast<std::size_t>(0));
  EXPECT_EQ(client.GetDeleteTuple().size(), static_cast<std::size_t>(0));
  EXPECT_EQ(client.GetAddDerivedLineage().size(), static_cast<std::size_t>(0));

  ASSERT_EQ(Status::OK, f.BootstrapTick());
  expected = {{{0}, {hash({0}), {1}}}};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  expected = {};
  EXPECT_EQ(f.Get<1>().Get(), expected);

  ASSERT_EQ(client.GetAddCollection().size(), static_cast<std::size_t>(2));
  ASSERT_EQ(client.GetAddRule().size(), static_cast<std::size_t>(3));
  ASSERT_EQ(client.GetInsertTuple().size(), static_cast<std::size_t>(1));
  EXPECT_EQ(client.GetInsertTuple()[0],
            InsertTupleTuple("t", 1, time_point(), {"0"}));
  ASSERT_EQ(client.GetDeleteTuple().size(), static_cast<std::size_t>(0));
  ASSERT_EQ(client.GetAddNetworkedLineage().size(),
            static_cast<std::size_t>(0));
  ASSERT_EQ(client.GetAddDerivedLineage().size(), static_cast<std::size_t>(0));

  MockClock::Advance(seconds(1));
  ASSERT_EQ(Status::OK, f.Tick());
  expected = {{{0}, {hash({0}), {1}}}, {{1}, {hash({1}), {4}}}};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  expected = {};
  EXPECT_EQ(f.Get<1>().Get(), expected);

  ASSERT_EQ(client.GetAddCollection().size(), static_cast<std::size_t>(2));
  ASSERT_EQ(client.GetAddRule().size(), static_cast<std::size_t>(3));
  ASSERT_EQ(client.GetInsertTuple().size(), static_cast<std::size_t>(3));
  EXPECT_EQ(client.GetInsertTuple()[1],
            InsertTupleTuple("s", 3, time_point(seconds(1)), {"1"}));
  EXPECT_EQ(client.GetInsertTuple()[2],
            InsertTupleTuple("t", 4, time_point(seconds(1)), {"1"}));
  ASSERT_EQ(client.GetDeleteTuple().size(), static_cast<std::size_t>(1));
  EXPECT_EQ(client.GetDeleteTuple()[0],
            DeleteTupleTuple("s", 5, time_point(seconds(1)), {"1"}));
  ASSERT_EQ(client.GetAddNetworkedLineage().size(),
            static_cast<std::size_t>(0));
  ASSERT_EQ(client.GetAddDerivedLineage().size(), static_cast<std::size_t>(2));
  EXPECT_EQ(client.GetAddDerivedLineage()[0],
            AddDerivedLineageTuple("t", hash({0}), 0, true,
                                   time_point(seconds(1)), "s", hash({1}), 3));
  EXPECT_EQ(client.GetAddDerivedLineage()[1],
            AddDerivedLineageTuple("s", hash({1}), 1, true,
                                   time_point(seconds(1)), "t", hash({1}), 4));

  MockClock::Advance(seconds(1));
  ASSERT_EQ(Status::OK, f.Tick());
  expected = {{{0}, {hash({0}), {1}}},
              {{1}, {hash({1}), {4, 7}}},
              {{2}, {hash({2}), {7}}}};
  EXPECT_EQ(f.Get<0>().Get(), expected);
  expected = {};
  EXPECT_EQ(f.Get<1>().Get(), expected);

  ASSERT_EQ(client.GetAddCollection().size(), static_cast<std::size_t>(2));
  ASSERT_EQ(client.GetAddRule().size(), static_cast<std::size_t>(3));
  ASSERT_EQ(client.GetInsertTuple().size(), static_cast<std::size_t>(7));
  EXPECT_EQ(client.GetInsertTuple()[3],
            InsertTupleTuple("s", 6, time_point(seconds(2)), {"1"}));
  EXPECT_EQ(client.GetInsertTuple()[4],
            InsertTupleTuple("s", 6, time_point(seconds(2)), {"2"}));
  EXPECT_EQ(client.GetInsertTuple()[5],
            InsertTupleTuple("t", 7, time_point(seconds(2)), {"1"}));
  EXPECT_EQ(client.GetInsertTuple()[6],
            InsertTupleTuple("t", 7, time_point(seconds(2)), {"2"}));
  ASSERT_EQ(client.GetDeleteTuple().size(), static_cast<std::size_t>(3));
  EXPECT_EQ(client.GetDeleteTuple()[1],
            DeleteTupleTuple("s", 8, time_point(seconds(2)), {"1"}));
  EXPECT_EQ(client.GetDeleteTuple()[2],
            DeleteTupleTuple("s", 8, time_point(seconds(2)), {"2"}));
  ASSERT_EQ(client.GetAddNetworkedLineage().size(),
            static_cast<std::size_t>(0));
  ASSERT_EQ(client.GetAddDerivedLineage().size(), static_cast<std::size_t>(6));
  EXPECT_EQ(client.GetAddDerivedLineage()[2],
            AddDerivedLineageTuple("t", hash({0}), 0, true,
                                   time_point(seconds(2)), "s", hash({1}), 6));
  EXPECT_EQ(client.GetAddDerivedLineage()[3],
            AddDerivedLineageTuple("t", hash({1}), 0, true,
                                   time_point(seconds(2)), "s", hash({2}), 6));
  EXPECT_EQ(client.GetAddDerivedLineage()[4],
            AddDerivedLineageTuple("s", hash({1}), 1, true,
                                   time_point(seconds(2)), "t", hash({1}), 7));
  EXPECT_EQ(client.GetAddDerivedLineage()[5],
            AddDerivedLineageTuple("s", hash({2}), 1, true,
                                   time_point(seconds(2)), "t", hash({2}), 7));
}

TEST(FluentExecutor, BlackBoxLineage) {
  zmq::context_t context(1);
  lineagedb::ConnectionConfig connection_config;
  auto fb_or =
      fluent<ldb::MockClient, Hash, ldb::MockToSql, MockPickler, MockClock>(
          "name", "inproc://yolo", &context, connection_config);
  ASSERT_EQ(Status::OK, fb_or.status());
  auto fe_or =
      fb_or.ConsumeValueOrDie()
          .channel<std::string, std::string, std::int64_t, int>(
              "f_request", {{"dst_addr", "src_addr", "id", "x"}})
          .channel<std::string, std::int64_t, int>("f_response",
                                                   {{"addr", "id", "y"}})
          .RegisterRules([](auto&, auto&) { return std::make_tuple(); });
  ASSERT_EQ(Status::OK, fe_or.status());
  auto f = fe_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, (f.RegisterBlackBoxLineage<0, 1>(
                            [](const std::string& time_inserted,
                               const std::string& x, const std::string& y) {
                              (void)time_inserted;
                              (void)x;
                              (void)y;
                              return "hello world";
                            })));

  using Client = ldb::MockClient<Hash, ldb::MockToSql, MockClock>;
  const Client& client = f.GetLineageDbClient();
  const std::vector<Client::RegisterBlackBoxLineageTuple>&
      black_box_lineage_tuples = client.GetRegisterBlackBoxLineage();
  ASSERT_EQ(black_box_lineage_tuples.size(), static_cast<std::size_t>(1));
  const Client::RegisterBlackBoxLineageTuple& t = black_box_lineage_tuples[0];
  EXPECT_EQ(CrunchWhitespace(std::get<0>(t)), CrunchWhitespace("f_response"));
  EXPECT_EQ(CrunchWhitespace(std::get<1>(t)[0]), CrunchWhitespace(R"(
    CREATE FUNCTION name_f_response_lineage_impl(integer, int, int)
    RETURNS TABLE(node_name text, collection_name text, hash bigint,
                  time_inserted integer)
    AS $$hello world$$ LANGUAGE SQL;
  )"));
  EXPECT_EQ(CrunchWhitespace(std::get<1>(t)[1]), CrunchWhitespace(R"(
    CREATE FUNCTION name_f_response_lineage(bigint)
    RETURNS TABLE(node_name text, collection_name text, hash bigint,
                  time_inserted integer)
    AS $$
      SELECT L.*
      FROM name_f_request Req,
           name_f_response Resp,
           name_f_response_lineage_impl(Req.time_inserted, Req.x, Resp.y) AS L
      WHERE Req.id = $1 AND Resp.id = $1
      UNION
      SELECT
        CAST('name' AS TEXT),
        CAST('f_request' AS TEXT),
        hash,
        time_inserted
      FROM name_f_request
      WHERE id = $1
    $$ LANGUAGE SQL;
  )"));
}

// TODO(mwhittaker): Test the lineage of a fluent program with communication
// and deletion.

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
