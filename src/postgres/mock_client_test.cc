#include "postgres/mock_client.h"

#include <cstddef>
#include <cstdint>

#include <string>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/hash_util.h"
#include "postgres/connection_config.h"
#include "postgres/mock_to_sql.h"
#include "postgres/to_sql.h"
#include "testing/test_util.h"

namespace fluent {
namespace postgres {
namespace detail {

struct MockCollection {
  std::string Name() const { return "c"; }
};

struct MockRuleTag {
  std::string ToDebugString() const { return "<="; }
};

struct MockRa {
  std::string ToDebugString() const { return "ra"; }
};

}  // namespace

TEST(MockClient, Init) {
  MockClient<Hash, MockToSql> client("", 42, ConnectionConfig());
  EXPECT_EQ(client.GetInit(), false);
  client.Init();
  EXPECT_EQ(client.GetInit(), true);
}

TEST(MockClient, AddCollection) {
  MockClient<Hash, MockToSql> client("", 42, ConnectionConfig());
  client.AddCollection<>("fee");
  client.AddCollection<int>("fi");
  client.AddCollection<int, bool>("fo");
  client.AddCollection<int, bool, char>("fum");

  using Pair = std::pair<std::string, std::vector<std::string>>;
  EXPECT_EQ(client.GetAddCollection()[0], Pair("fee", {}));
  EXPECT_EQ(client.GetAddCollection()[1], Pair("fi", {"int"}));
  EXPECT_EQ(client.GetAddCollection()[2], Pair("fo", {"int", "bool"}));
  EXPECT_EQ(client.GetAddCollection()[3], Pair("fum", {"int", "bool", "char"}));
}

TEST(MockClient, AddRule) {
  MockClient<Hash, MockToSql> client("", 42, ConnectionConfig());
  detail::MockCollection mock_collection;
  auto rule = std::make_tuple(&mock_collection, detail::MockRuleTag(),
                              detail::MockRa());
  client.AddRule(0, rule);
  client.AddRule(1, rule);
  client.AddRule(2, rule);

  using Pair = std::pair<std::size_t, std::string>;
  EXPECT_EQ(client.GetAddRule()[0], Pair(0, "c <= ra"));
  EXPECT_EQ(client.GetAddRule()[1], Pair(1, "c <= ra"));
  EXPECT_EQ(client.GetAddRule()[2], Pair(2, "c <= ra"));
}

TEST(MockClient, InsertTuple) {
  MockClient<Hash, MockToSql> client("", 42, ConnectionConfig());
  client.InsertTuple("a", 0, std::tuple<>{});
  client.InsertTuple("b", 1, std::tuple<int>{10});
  client.InsertTuple("c", 2, std::tuple<int, char, bool>{42, 'x', false});

  using Triple = std::tuple<std::string, int, std::vector<std::string>>;
  EXPECT_EQ(client.GetInsertTuple()[0], Triple("a", 0, {}));
  EXPECT_EQ(client.GetInsertTuple()[1], Triple("b", 1, {"10"}));
  EXPECT_EQ(client.GetInsertTuple()[2], Triple("c", 2, {"42", "x", "false"}));
}

TEST(MockClient, DeleteTuple) {
  MockClient<Hash, MockToSql> client("", 42, ConnectionConfig());
  client.DeleteTuple("a", 0, std::tuple<>{});
  client.DeleteTuple("b", 1, std::tuple<int>{10});
  client.DeleteTuple("c", 2, std::tuple<int, char, bool>{42, 'x', false});

  using Triple = std::tuple<std::string, int, std::vector<std::string>>;
  EXPECT_EQ(client.GetDeleteTuple()[0], Triple("a", 0, {}));
  EXPECT_EQ(client.GetDeleteTuple()[1], Triple("b", 1, {"10"}));
  EXPECT_EQ(client.GetDeleteTuple()[2], Triple("c", 2, {"42", "x", "false"}));
}

TEST(MockClient, AddNetworkedLineage) {
  MockClient<Hash, MockToSql> client("", 42, ConnectionConfig());
  client.AddNetworkedLineage(0, 1, "2", 3, 4);
  client.AddNetworkedLineage(10, 11, "12", 13, 14);
  client.AddNetworkedLineage(20, 21, "22", 23, 24);

  using Tuple = std::tuple<std::size_t, int, std::string, std::size_t, int>;
  EXPECT_EQ(client.GetAddNetworkedLineage()[0], Tuple(0, 1, "2", 3, 4));
  EXPECT_EQ(client.GetAddNetworkedLineage()[1], Tuple(10, 11, "12", 13, 14));
  EXPECT_EQ(client.GetAddNetworkedLineage()[2], Tuple(20, 21, "22", 23, 24));
}

TEST(MockClient, AddDerivedLineage) {
  MockClient<Hash, MockToSql> client("", 42, ConnectionConfig());
  client.AddDerivedLineage("0", 1, 2, true, "3", 4, 5);
  client.AddDerivedLineage("10", 11, 12, true, "13", 14, 15);
  client.AddDerivedLineage("20", 21, 22, true, "23", 24, 25);

  using Tuple = std::tuple<std::string, std::size_t, int, bool, std::string,
                           std::size_t, int>;
  EXPECT_EQ(client.GetAddDerivedLineage()[0],
            Tuple("0", 1, 2, true, "3", 4, 5));
  EXPECT_EQ(client.GetAddDerivedLineage()[1],
            Tuple("10", 11, 12, true, "13", 14, 15));
  EXPECT_EQ(client.GetAddDerivedLineage()[2],
            Tuple("20", 21, 22, true, "23", 24, 25));
}

}  // namespace postgres
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
