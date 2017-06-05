#include "lineagedb/mock_client.h"

#include <cstddef>
#include <cstdint>

#include <string>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/hash_util.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/mock_to_sql.h"
#include "lineagedb/to_sql.h"
#include "testing/test_util.h"

namespace fluent {
namespace lineagedb {

TEST(MockClient, Init) {
  MockClient<Hash, MockToSql> client("", 42, "", ConnectionConfig());
  EXPECT_EQ(client.GetInit(), false);
  client.Init();
  EXPECT_EQ(client.GetInit(), true);
}

TEST(MockClient, AddCollection) {
  MockClient<Hash, MockToSql> client("", 42, "", ConnectionConfig());
  client.AddCollection<>("fee", "Fee", {{}});
  client.AddCollection<int>("fi", "Fi", {{"x"}});
  client.AddCollection<int, bool>("fo", "Fo", {{"x", "y"}});
  client.AddCollection<int, bool, char>("fum", "Fum", {{"x", "y", "z"}});

  using Tuple = MockClient<Hash, MockToSql>::AddCollectionTuple;
  EXPECT_EQ(client.GetAddCollection()[0], Tuple("fee", "Fee", {}, {}));
  EXPECT_EQ(client.GetAddCollection()[1], Tuple("fi", "Fi", {"x"}, {"int"}));
  EXPECT_EQ(client.GetAddCollection()[2],
            Tuple("fo", "Fo", {"x", "y"}, {"int", "bool"}));
  EXPECT_EQ(client.GetAddCollection()[3],
            Tuple("fum", "Fum", {"x", "y", "z"}, {"int", "bool", "char"}));
}

TEST(MockClient, AddRule) {
  MockClient<Hash, MockToSql> client("", 42, "", ConnectionConfig());
  client.AddRule(0, true, "foo");
  client.AddRule(1, true, "bar");
  client.AddRule(2, true, "baz");
  client.AddRule(0, false, "foo");
  client.AddRule(1, false, "bar");
  client.AddRule(2, false, "baz");

  using Tuple = MockClient<Hash, MockToSql>::AddRuleTuple;
  EXPECT_EQ(client.GetAddRule()[0], Tuple(0, true, "foo"));
  EXPECT_EQ(client.GetAddRule()[1], Tuple(1, true, "bar"));
  EXPECT_EQ(client.GetAddRule()[2], Tuple(2, true, "baz"));
  EXPECT_EQ(client.GetAddRule()[3], Tuple(0, false, "foo"));
  EXPECT_EQ(client.GetAddRule()[4], Tuple(1, false, "bar"));
  EXPECT_EQ(client.GetAddRule()[5], Tuple(2, false, "baz"));
}

TEST(MockClient, InsertTuple) {
  MockClient<Hash, MockToSql> client("", 42, "", ConnectionConfig());
  client.InsertTuple("a", 0, std::tuple<>{});
  client.InsertTuple("b", 1, std::tuple<int>{10});
  client.InsertTuple("c", 2, std::tuple<int, char, bool>{42, 'x', false});

  using Tuple = MockClient<Hash, MockToSql>::InsertTupleTuple;
  EXPECT_EQ(client.GetInsertTuple()[0], Tuple("a", 0, {}));
  EXPECT_EQ(client.GetInsertTuple()[1], Tuple("b", 1, {"10"}));
  EXPECT_EQ(client.GetInsertTuple()[2], Tuple("c", 2, {"42", "x", "false"}));
}

TEST(MockClient, DeleteTuple) {
  MockClient<Hash, MockToSql> client("", 42, "", ConnectionConfig());
  client.DeleteTuple("a", 0, std::tuple<>{});
  client.DeleteTuple("b", 1, std::tuple<int>{10});
  client.DeleteTuple("c", 2, std::tuple<int, char, bool>{42, 'x', false});

  using Tuple = MockClient<Hash, MockToSql>::DeleteTupleTuple;
  EXPECT_EQ(client.GetDeleteTuple()[0], Tuple("a", 0, {}));
  EXPECT_EQ(client.GetDeleteTuple()[1], Tuple("b", 1, {"10"}));
  EXPECT_EQ(client.GetDeleteTuple()[2], Tuple("c", 2, {"42", "x", "false"}));
}

TEST(MockClient, AddNetworkedLineage) {
  MockClient<Hash, MockToSql> client("", 42, "", ConnectionConfig());
  client.AddNetworkedLineage(0, 1, "2", 3, 4);
  client.AddNetworkedLineage(10, 11, "12", 13, 14);
  client.AddNetworkedLineage(20, 21, "22", 23, 24);

  using Tuple = MockClient<Hash, MockToSql>::AddNetworkedLineageTuple;
  EXPECT_EQ(client.GetAddNetworkedLineage()[0], Tuple(0, 1, "2", 3, 4));
  EXPECT_EQ(client.GetAddNetworkedLineage()[1], Tuple(10, 11, "12", 13, 14));
  EXPECT_EQ(client.GetAddNetworkedLineage()[2], Tuple(20, 21, "22", 23, 24));
}

TEST(MockClient, AddDerivedLineage) {
  MockClient<Hash, MockToSql> client("", 42, "", ConnectionConfig());
  client.AddDerivedLineage("0", 1, 2, true, "3", 4, 5);
  client.AddDerivedLineage("10", 11, 12, true, "13", 14, 15);
  client.AddDerivedLineage("20", 21, 22, true, "23", 24, 25);

  using Tuple = MockClient<Hash, MockToSql>::AddDerivedLineageTuple;
  EXPECT_EQ(client.GetAddDerivedLineage()[0],
            Tuple("0", 1, 2, true, "3", 4, 5));
  EXPECT_EQ(client.GetAddDerivedLineage()[1],
            Tuple("10", 11, 12, true, "13", 14, 15));
  EXPECT_EQ(client.GetAddDerivedLineage()[2],
            Tuple("20", 21, 22, true, "23", 24, 25));
}

}  // namespace lineagedb
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
