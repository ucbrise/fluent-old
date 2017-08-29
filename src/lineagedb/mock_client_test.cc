#include "lineagedb/mock_client.h"

#include <cstddef>
#include <cstdint>

#include <chrono>
#include <memory>
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

using common::Hash;
using testing::MockClock;

TEST(MockClient, AddCollection) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  common::StatusOr<std::unique_ptr<Client>> client_or =
      Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(common::Status::OK, client_or.status());
  std::unique_ptr<Client> client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(common::Status::OK, client->AddCollection<>("fee", "Fee", {{}}));
  ASSERT_EQ(common::Status::OK,
            client->AddCollection<int>("fi", "Fi", {{"x"}}));
  ASSERT_EQ(common::Status::OK,
            (client->AddCollection<int, bool>("fo", "Fo", {{"x", "y"}})));
  ASSERT_EQ(common::Status::OK, (client->AddCollection<int, bool, char>(
                                    "fum", "Fum", {{"x", "y", "z"}})));

  using Tuple = MockClient<Hash, MockToSql, MockClock>::AddCollectionTuple;
  ASSERT_EQ(client->GetAddCollection().size(), static_cast<std::size_t>(4));
  EXPECT_EQ(client->GetAddCollection()[0], Tuple("fee", "Fee", {}, {}));
  EXPECT_EQ(client->GetAddCollection()[1], Tuple("fi", "Fi", {"x"}, {"int"}));
  EXPECT_EQ(client->GetAddCollection()[2],
            Tuple("fo", "Fo", {"x", "y"}, {"int", "bool"}));
  EXPECT_EQ(client->GetAddCollection()[3],
            Tuple("fum", "Fum", {"x", "y", "z"}, {"int", "bool", "char"}));
}

TEST(MockClient, AddRule) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  common::StatusOr<std::unique_ptr<Client>> client_or =
      Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(common::Status::OK, client_or.status());
  std::unique_ptr<Client> client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(common::Status::OK, client->AddRule(0, true, "foo"));
  ASSERT_EQ(common::Status::OK, client->AddRule(1, true, "bar"));
  ASSERT_EQ(common::Status::OK, client->AddRule(2, true, "baz"));
  ASSERT_EQ(common::Status::OK, client->AddRule(0, false, "foo"));
  ASSERT_EQ(common::Status::OK, client->AddRule(1, false, "bar"));
  ASSERT_EQ(common::Status::OK, client->AddRule(2, false, "baz"));

  using Tuple = MockClient<Hash, MockToSql, MockClock>::AddRuleTuple;
  ASSERT_EQ(client->GetAddRule().size(), static_cast<std::size_t>(6));
  EXPECT_EQ(client->GetAddRule()[0], Tuple(0, true, "foo"));
  EXPECT_EQ(client->GetAddRule()[1], Tuple(1, true, "bar"));
  EXPECT_EQ(client->GetAddRule()[2], Tuple(2, true, "baz"));
  EXPECT_EQ(client->GetAddRule()[3], Tuple(0, false, "foo"));
  EXPECT_EQ(client->GetAddRule()[4], Tuple(1, false, "bar"));
  EXPECT_EQ(client->GetAddRule()[5], Tuple(2, false, "baz"));
}

TEST(MockClient, InsertTuple) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  using time_point = std::chrono::time_point<MockClock>;

  time_point zero_sec = time_point(std::chrono::seconds(0));
  time_point one_sec = time_point(std::chrono::seconds(1));
  time_point two_sec = time_point(std::chrono::seconds(2));

  common::StatusOr<std::unique_ptr<Client>> client_or =
      Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(common::Status::OK, client_or.status());
  std::unique_ptr<Client> client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(common::Status::OK,
            (client->InsertTuple("a", 0, zero_sec, std::tuple<>{})));
  ASSERT_EQ(common::Status::OK,
            (client->InsertTuple("b", 1, one_sec, std::tuple<int>{10})));
  ASSERT_EQ(common::Status::OK,
            (client->InsertTuple("c", 2, two_sec,
                                 std::tuple<int, char, bool>{42, 'x', false})));

  using Tuple = MockClient<Hash, MockToSql, MockClock>::InsertTupleTuple;
  ASSERT_EQ(client->GetInsertTuple().size(), static_cast<std::size_t>(3));
  EXPECT_EQ(client->GetInsertTuple()[0], Tuple("a", 0, zero_sec, {}));
  EXPECT_EQ(client->GetInsertTuple()[1], Tuple("b", 1, one_sec, {"10"}));
  EXPECT_EQ(client->GetInsertTuple()[2],
            Tuple("c", 2, two_sec, {"42", "x", "false"}));
}

TEST(MockClient, DeleteTuple) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  using time_point = std::chrono::time_point<MockClock>;

  time_point zero_sec = time_point(std::chrono::seconds(0));
  time_point one_sec = time_point(std::chrono::seconds(1));
  time_point two_sec = time_point(std::chrono::seconds(2));

  common::StatusOr<std::unique_ptr<Client>> client_or =
      Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(common::Status::OK, client_or.status());
  std::unique_ptr<Client> client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(common::Status::OK,
            (client->DeleteTuple("a", 0, zero_sec, std::tuple<>{})));
  ASSERT_EQ(common::Status::OK,
            (client->DeleteTuple("b", 1, one_sec, std::tuple<int>{10})));
  ASSERT_EQ(common::Status::OK,
            (client->DeleteTuple("c", 2, two_sec,
                                 std::tuple<int, char, bool>{42, 'x', false})));

  using Tuple = MockClient<Hash, MockToSql, MockClock>::DeleteTupleTuple;
  ASSERT_EQ(client->GetDeleteTuple().size(), static_cast<std::size_t>(3));
  EXPECT_EQ(client->GetDeleteTuple()[0], Tuple("a", 0, zero_sec, {}));
  EXPECT_EQ(client->GetDeleteTuple()[1], Tuple("b", 1, one_sec, {"10"}));
  EXPECT_EQ(client->GetDeleteTuple()[2],
            Tuple("c", 2, two_sec, {"42", "x", "false"}));
}

TEST(MockClient, AddNetworkedLineage) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  common::StatusOr<std::unique_ptr<Client>> client_or =
      Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(common::Status::OK, client_or.status());
  std::unique_ptr<Client> client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(common::Status::OK, (client->AddNetworkedLineage(0, 1, "2", 3, 4)));
  ASSERT_EQ(common::Status::OK,
            (client->AddNetworkedLineage(10, 11, "12", 13, 14)));
  ASSERT_EQ(common::Status::OK,
            (client->AddNetworkedLineage(20, 21, "22", 23, 24)));

  using Tuple =
      MockClient<Hash, MockToSql, MockClock>::AddNetworkedLineageTuple;
  ASSERT_EQ(client->GetAddNetworkedLineage().size(),
            static_cast<std::size_t>(3));
  EXPECT_EQ(client->GetAddNetworkedLineage()[0], Tuple(0, 1, "2", 3, 4));
  EXPECT_EQ(client->GetAddNetworkedLineage()[1], Tuple(10, 11, "12", 13, 14));
  EXPECT_EQ(client->GetAddNetworkedLineage()[2], Tuple(20, 21, "22", 23, 24));
}

TEST(MockClient, AddDerivedLineage) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  using time_point = std::chrono::time_point<MockClock>;
  using std::chrono::seconds;

  common::StatusOr<std::unique_ptr<Client>> client_or =
      Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(common::Status::OK, client_or.status());
  std::unique_ptr<Client> client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(common::Status::OK,
            (client->AddDerivedLineage(LocalTupleId{"0", 1, 2},          //
                                       3, true, time_point(seconds(1)),  //
                                       LocalTupleId{"4", 5, 6})));
  ASSERT_EQ(common::Status::OK,
            (client->AddDerivedLineage(LocalTupleId{"10", 11, 12},  //
                                       13, true,
                                       time_point(seconds(2)),  //
                                       LocalTupleId{"14", 15, 16})));
  ASSERT_EQ(common::Status::OK,
            (client->AddDerivedLineage(LocalTupleId{"20", 21, 22},  //
                                       23, true,
                                       time_point(seconds(3)),  //
                                       LocalTupleId{"24", 25, 26})));

  using Tuple = MockClient<Hash, MockToSql, MockClock>::AddDerivedLineageTuple;
  ASSERT_EQ(client->GetAddDerivedLineage().size(), static_cast<std::size_t>(3));
  EXPECT_EQ(client->GetAddDerivedLineage()[0],
            Tuple(LocalTupleId{"0", 1, 2},          //
                  3, true, time_point(seconds(1)),  //
                  LocalTupleId{"4", 5, 6}));
  EXPECT_EQ(client->GetAddDerivedLineage()[1],
            Tuple(LocalTupleId{"10", 11, 12},        //
                  13, true, time_point(seconds(2)),  //
                  LocalTupleId{"14", 15, 16}));
  EXPECT_EQ(client->GetAddDerivedLineage()[2],
            Tuple(LocalTupleId{"20", 21, 22},        //
                  23, true, time_point(seconds(3)),  //
                  LocalTupleId{"24", 25, 26}));
}

TEST(MockClient, RegisterBlackBoxLineage) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  common::StatusOr<std::unique_ptr<Client>> client_or =
      Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(common::Status::OK, client_or.status());
  std::unique_ptr<Client> client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(common::Status::OK, (client->RegisterBlackBoxLineage("foo", {})));
  ASSERT_EQ(common::Status::OK,
            (client->RegisterBlackBoxLineage("bar", {"1"})));
  ASSERT_EQ(common::Status::OK,
            (client->RegisterBlackBoxLineage("baz", {"1", "2"})));

  using Tuple = Client::RegisterBlackBoxLineageTuple;
  ASSERT_EQ(client->GetRegisterBlackBoxLineage().size(),
            static_cast<std::size_t>(3));
  EXPECT_EQ(client->GetRegisterBlackBoxLineage()[0], Tuple("foo", {}));
  EXPECT_EQ(client->GetRegisterBlackBoxLineage()[1], Tuple("bar", {"1"}));
  EXPECT_EQ(client->GetRegisterBlackBoxLineage()[2], Tuple("baz", {"1", "2"}));
}

TEST(MockClient, RegisterBlackBoxPythonLineageScript) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  common::StatusOr<std::unique_ptr<Client>> client_or =
      Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(common::Status::OK, client_or.status());
  std::unique_ptr<Client> client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(
      common::Status::OK,
      (client->RegisterBlackBoxPythonLineageScript("bonnie\nand\nclyde")));
  ASSERT_EQ(common::Status::OK,
            (client->RegisterBlackBoxPythonLineageScript("bert\nand\nernie")));

  using Tuple = Client::RegisterBlackBoxPythonLineageScriptTuple;
  ASSERT_EQ(client->GetRegisterBlackBoxPythonLineageScript().size(),
            static_cast<std::size_t>(2));
  EXPECT_EQ(client->GetRegisterBlackBoxPythonLineageScript()[0],
            Tuple("bonnie\nand\nclyde"));
  EXPECT_EQ(client->GetRegisterBlackBoxPythonLineageScript()[1],
            Tuple("bert\nand\nernie"));
}

TEST(MockClient, RegisterBlackBoxPythonLineage) {
  using Client = MockClient<Hash, MockToSql, MockClock>;
  common::StatusOr<std::unique_ptr<Client>> client_or =
      Client::Make("", 42, "", ConnectionConfig());
  ASSERT_EQ(common::Status::OK, client_or.status());
  std::unique_ptr<Client> client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(common::Status::OK,
            (client->RegisterBlackBoxPythonLineage("foo", "get")));
  ASSERT_EQ(common::Status::OK,
            (client->RegisterBlackBoxPythonLineage("bar", "set")));

  using Tuple = Client::RegisterBlackBoxPythonLineageTuple;
  ASSERT_EQ(client->GetRegisterBlackBoxPythonLineage().size(),
            static_cast<std::size_t>(2));
  EXPECT_EQ(client->GetRegisterBlackBoxPythonLineage()[0], Tuple("foo", "get"));
  EXPECT_EQ(client->GetRegisterBlackBoxPythonLineage()[1], Tuple("bar", "set"));
}

}  // namespace lineagedb
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
