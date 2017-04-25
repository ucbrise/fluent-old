#include "lineagedb/mock_client.h"

#include <cstddef>
#include <cstdint>

#include <chrono>
#include <string>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/hash_util.h"
#include "common/status.h"
#include "common/status_or.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/mock_to_sql.h"
#include "lineagedb/to_sql.h"
#include "testing/mock_clock.h"
#include "testing/test_util.h"

namespace fluent {
namespace lineagedb {

TEST(MockClient, AddCollection) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  StatusOr<Client> client_or = Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, client.AddCollection<>("fee", "Fee", {{}}));
  ASSERT_EQ(Status::OK, client.AddCollection<int>("fi", "Fi", {{"x"}}));
  ASSERT_EQ(Status::OK,
            (client.AddCollection<int, bool>("fo", "Fo", {{"x", "y"}})));
  ASSERT_EQ(Status::OK, (client.AddCollection<int, bool, char>(
                            "fum", "Fum", {{"x", "y", "z"}})));

  using Tuple = MockClient<Hash, MockToSql, MockClock>::AddCollectionTuple;
  ASSERT_EQ(client.GetAddCollection().size(), static_cast<std::size_t>(4));
  EXPECT_EQ(client.GetAddCollection()[0], Tuple("fee", "Fee", {}, {}));
  EXPECT_EQ(client.GetAddCollection()[1], Tuple("fi", "Fi", {"x"}, {"int"}));
  EXPECT_EQ(client.GetAddCollection()[2],
            Tuple("fo", "Fo", {"x", "y"}, {"int", "bool"}));
  EXPECT_EQ(client.GetAddCollection()[3],
            Tuple("fum", "Fum", {"x", "y", "z"}, {"int", "bool", "char"}));
}

TEST(MockClient, AddRule) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  StatusOr<Client> client_or = Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, client.AddRule(0, true, "foo"));
  ASSERT_EQ(Status::OK, client.AddRule(1, true, "bar"));
  ASSERT_EQ(Status::OK, client.AddRule(2, true, "baz"));
  ASSERT_EQ(Status::OK, client.AddRule(0, false, "foo"));
  ASSERT_EQ(Status::OK, client.AddRule(1, false, "bar"));
  ASSERT_EQ(Status::OK, client.AddRule(2, false, "baz"));

  using Tuple = MockClient<Hash, MockToSql, MockClock>::AddRuleTuple;
  ASSERT_EQ(client.GetAddRule().size(), static_cast<std::size_t>(6));
  EXPECT_EQ(client.GetAddRule()[0], Tuple(0, true, "foo"));
  EXPECT_EQ(client.GetAddRule()[1], Tuple(1, true, "bar"));
  EXPECT_EQ(client.GetAddRule()[2], Tuple(2, true, "baz"));
  EXPECT_EQ(client.GetAddRule()[3], Tuple(0, false, "foo"));
  EXPECT_EQ(client.GetAddRule()[4], Tuple(1, false, "bar"));
  EXPECT_EQ(client.GetAddRule()[5], Tuple(2, false, "baz"));
}

TEST(MockClient, InsertTuple) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  using time_point = std::chrono::time_point<MockClock>;

  time_point zero_sec = time_point(std::chrono::seconds(0));
  time_point one_sec = time_point(std::chrono::seconds(1));
  time_point two_sec = time_point(std::chrono::seconds(2));

  StatusOr<Client> client_or = Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, (client.InsertTuple("a", 0, zero_sec, std::tuple<>{})));
  ASSERT_EQ(Status::OK,
            (client.InsertTuple("b", 1, one_sec, std::tuple<int>{10})));
  ASSERT_EQ(Status::OK,
            (client.InsertTuple("c", 2, two_sec,
                                std::tuple<int, char, bool>{42, 'x', false})));

  using Tuple = MockClient<Hash, MockToSql, MockClock>::InsertTupleTuple;
  ASSERT_EQ(client.GetInsertTuple().size(), static_cast<std::size_t>(3));
  EXPECT_EQ(client.GetInsertTuple()[0], Tuple("a", 0, zero_sec, {}));
  EXPECT_EQ(client.GetInsertTuple()[1], Tuple("b", 1, one_sec, {"10"}));
  EXPECT_EQ(client.GetInsertTuple()[2],
            Tuple("c", 2, two_sec, {"42", "x", "false"}));
}

TEST(MockClient, DeleteTuple) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  using time_point = std::chrono::time_point<MockClock>;

  time_point zero_sec = time_point(std::chrono::seconds(0));
  time_point one_sec = time_point(std::chrono::seconds(1));
  time_point two_sec = time_point(std::chrono::seconds(2));

  StatusOr<Client> client_or = Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, (client.DeleteTuple("a", 0, zero_sec, std::tuple<>{})));
  ASSERT_EQ(Status::OK,
            (client.DeleteTuple("b", 1, one_sec, std::tuple<int>{10})));
  ASSERT_EQ(Status::OK,
            (client.DeleteTuple("c", 2, two_sec,
                                std::tuple<int, char, bool>{42, 'x', false})));

  using Tuple = MockClient<Hash, MockToSql, MockClock>::DeleteTupleTuple;
  ASSERT_EQ(client.GetDeleteTuple().size(), static_cast<std::size_t>(3));
  EXPECT_EQ(client.GetDeleteTuple()[0], Tuple("a", 0, zero_sec, {}));
  EXPECT_EQ(client.GetDeleteTuple()[1], Tuple("b", 1, one_sec, {"10"}));
  EXPECT_EQ(client.GetDeleteTuple()[2],
            Tuple("c", 2, two_sec, {"42", "x", "false"}));
}

TEST(MockClient, AddNetworkedLineage) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  StatusOr<Client> client_or = Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, (client.AddNetworkedLineage(0, 1, "2", 3, 4)));
  ASSERT_EQ(Status::OK, (client.AddNetworkedLineage(10, 11, "12", 13, 14)));
  ASSERT_EQ(Status::OK, (client.AddNetworkedLineage(20, 21, "22", 23, 24)));

  using Tuple =
      MockClient<Hash, MockToSql, MockClock>::AddNetworkedLineageTuple;
  ASSERT_EQ(client.GetAddNetworkedLineage().size(),
            static_cast<std::size_t>(3));
  EXPECT_EQ(client.GetAddNetworkedLineage()[0], Tuple(0, 1, "2", 3, 4));
  EXPECT_EQ(client.GetAddNetworkedLineage()[1], Tuple(10, 11, "12", 13, 14));
  EXPECT_EQ(client.GetAddNetworkedLineage()[2], Tuple(20, 21, "22", 23, 24));
}

TEST(MockClient, AddDerivedLineage) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  using time_point = std::chrono::time_point<MockClock>;
  using std::chrono::seconds;

  StatusOr<Client> client_or = Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK,
            (client.AddDerivedLineage("0", 1, 2, true, time_point(seconds(1)),
                                      "3", 4, 5)));
  ASSERT_EQ(Status::OK,
            (client.AddDerivedLineage("10", 11, 12, true,
                                      time_point(seconds(2)), "13", 14, 15)));
  ASSERT_EQ(Status::OK,
            (client.AddDerivedLineage("20", 21, 22, true,
                                      time_point(seconds(3)), "23", 24, 25)));

  using Tuple = MockClient<Hash, MockToSql, MockClock>::AddDerivedLineageTuple;
  ASSERT_EQ(client.GetAddDerivedLineage().size(), static_cast<std::size_t>(3));
  EXPECT_EQ(client.GetAddDerivedLineage()[0],
            Tuple("0", 1, 2, true, time_point(seconds(1)), "3", 4, 5));
  EXPECT_EQ(client.GetAddDerivedLineage()[1],
            Tuple("10", 11, 12, true, time_point(seconds(2)), "13", 14, 15));
  EXPECT_EQ(client.GetAddDerivedLineage()[2],
            Tuple("20", 21, 22, true, time_point(seconds(3)), "23", 24, 25));
}

TEST(MockClient, Exec) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  StatusOr<Client> client_or = Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, client.Exec("a cat"));
  ASSERT_EQ(Status::OK, client.Exec("in the hat"));
  ASSERT_EQ(Status::OK, client.Exec("ate green eggs and ham"));

  using Tuple = MockClient<Hash, MockToSql, MockClock>::ExecTuple;
  ASSERT_EQ(client.GetExec().size(), static_cast<std::size_t>(3));
  EXPECT_EQ(client.GetExec()[0], Tuple("a cat"));
  EXPECT_EQ(client.GetExec()[1], Tuple("in the hat"));
  EXPECT_EQ(client.GetExec()[2], Tuple("ate green eggs and ham"));
}

}  // namespace lineagedb
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
