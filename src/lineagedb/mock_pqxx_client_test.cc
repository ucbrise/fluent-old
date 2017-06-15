#include "lineagedb/mock_pqxx_client.h"

#include <cstddef>
#include <cstdint>

#include <string>
#include <tuple>

#include "fmt/format.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/hash_util.h"
#include "fluent/local_tuple_id.h"
#include "lineagedb/mock_to_sql.h"
#include "lineagedb/to_sql.h"
#include "testing/mock_clock.h"
#include "testing/test_util.h"

// These unit tests are all whitebox tests that you will probably have to change
// if you change the implementation of PqxxClient. They are mostly here just to
// make sure things compile and to catch silly trivial bugs.

namespace fluent {
namespace lineagedb {

TEST(MockPqxxClient, Init) {
  using Client = MockPqxxClient<Hash, ToSql, MockClock>;

  ConnectionConfig c;
  StatusOr<Client> client_or = Client::Make("name", 9001, "127.0.0.1", c);
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();
  ASSERT_EQ(queries.size(), static_cast<std::size_t>(2));
  ExpectStringsEqualIgnoreWhiteSpace(queries[0].second, R"(
    INSERT INTO Nodes (id, name, address, python_lineage_script)
    VALUES (9001, 'name', '127.0.0.1', NULL);
  )");
  ExpectStringsEqualIgnoreWhiteSpace(queries[1].second, R"(
    CREATE TABLE name_lineage (
      dep_node_id          bigint                    NOT NULL,
      dep_collection_name  text                      NOT NULL,
      dep_tuple_hash       bigint                    NOT NULL,
      dep_time             bigint                    NOT NULL,
      rule_number          integer,
      inserted             boolean                   NOT NULL,
      physical_time        timestamp with time zone,
      collection_name      text                      NOT NULL,
      tuple_hash           bigint                    NOT NULL,
      time                 integer                   NOT NULL
    );
  )");
}

TEST(MockPqxxClient, AddCollection) {
  using Client = MockPqxxClient<Hash, ToSql, MockClock>;

  ConnectionConfig c;
  StatusOr<Client> client_or = Client::Make("name", 9001, "127.0.0.1", c);
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, (client.AddCollection<int, char, bool>(
                            "t", "Table", {{"x", "c", "b"}})));

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();
  ASSERT_EQ(queries.size(), static_cast<std::size_t>(4));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, R"(
    INSERT INTO Collections (node_id, collection_name, collection_type,
                             column_names, lineage_type, python_lineage_method)
    VALUES (9001, 't', 'Table', ARRAY['x', 'c', 'b'], 'regular', NULL);
  )");
  ExpectStringsEqualIgnoreWhiteSpace(queries[3].second, R"(
    CREATE TABLE name_t (
      hash bigint  NOT NULL,
      time_inserted integer NOT NULL,
      time_deleted integer,
      physical_time_inserted timestamp with time zone NOT NULL,
      physical_time_deleted timestamp with time zone,
      x integer NOT NULL,
      c char(1) NOT NULL,
      b boolean NOT NULL,
      PRIMARY KEY (hash, time_inserted)
    );
  )");
}

TEST(MockPqxxClient, AddRule) {
  using Client = MockPqxxClient<Hash, ToSql, MockClock>;

  ConnectionConfig c;
  StatusOr<Client> client_or = Client::Make("name", 9001, "127.0.0.1", c);
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, client.AddRule(0, true, "foo"));

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();
  ASSERT_EQ(queries.size(), static_cast<std::size_t>(3));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, R"(
    INSERT INTO Rules (node_id, rule_number, is_bootstrap, rule)
    VALUES (9001, 0, true, 'foo');
  )");
}

TEST(MockPqxxClient, InsertTuple) {
  using Client = MockPqxxClient<Hash, MockToSql, MockClock>;
  using time_point = std::chrono::time_point<MockClock>;
  using tuple_t = std::tuple<int, bool, char>;

  ConnectionConfig c;
  tuple_t t = {1, true, 'a'};

  StatusOr<Client> client_or = Client::Make("name", 9001, "127.0.0.1", c);
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, client.InsertTuple(
                            "t", 42, time_point(std::chrono::seconds(43)), t));

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();
  std::int64_t hash = detail::size_t_to_int64(Hash<tuple_t>()(t));

  ASSERT_EQ(queries.size(), static_cast<std::size_t>(3));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, fmt::format(R"(
    INSERT INTO name_t
    VALUES ({}, 42, NULL, epoch + 43 seconds, NULL, 1, true, a);
  )",
                                                                    hash));
}

TEST(MockPqxxClient, DeleteTuple) {
  using Client = MockPqxxClient<Hash, MockToSql, MockClock>;
  using time_point = std::chrono::time_point<MockClock>;
  using tuple_t = std::tuple<int, bool, char>;

  ConnectionConfig c;
  tuple_t t = {1, true, 'a'};

  StatusOr<Client> client_or = Client::Make("name", 9001, "127.0.0.1", c);
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, client.DeleteTuple(
                            "t", 42, time_point(std::chrono::seconds(43)), t));

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();
  std::int64_t hash = detail::size_t_to_int64(Hash<tuple_t>()(t));

  ASSERT_EQ(queries.size(), static_cast<std::size_t>(3));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, fmt::format(R"(
    UPDATE name_t
    SET time_deleted = 42, physical_time_deleted = epoch + 43 seconds
    WHERE hash = {} AND time_deleted IS NULL;
  )",
                                                                    hash));
}

TEST(MockPqxxClient, AddNetworkedLineage) {
  using Client = MockPqxxClient<Hash, ToSql, MockClock>;

  ConnectionConfig c;
  StatusOr<Client> client_or = Client::Make("name", 9001, "127.0.0.1", c);
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, client.AddNetworkedLineage(0, 1, "foo", 2, 3));

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();

  ASSERT_EQ(queries.size(), static_cast<std::size_t>(3));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, fmt::format(R"(
    INSERT INTO name_lineage (dep_node_id, dep_collection_name, dep_tuple_hash,
                              dep_time, rule_number, inserted, collection_name,
                              tuple_hash, time)
    VALUES (0, 'foo', 2, 1, NULL, true, 'foo', 2, 3);
  )"));
}

TEST(MockPqxxClient, AddDerivedLineage) {
  using Client = MockPqxxClient<Hash, MockToSql, MockClock>;

  ConnectionConfig c;
  StatusOr<Client> client_or = Client::Make("name", 9001, "127.0.0.1", c);
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK,
            client.AddDerivedLineage(LocalTupleId{"foo", 1, 2}, 3, true,
                                     std::chrono::time_point<MockClock>(),
                                     LocalTupleId{"bar", 4, 5}));

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();

  ASSERT_EQ(queries.size(), static_cast<std::size_t>(3));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, fmt::format(R"(
    INSERT INTO name_lineage (dep_node_id, dep_collection_name, dep_tuple_hash,
                              dep_time, rule_number, inserted, physical_time,
                              collection_name, tuple_hash, time)
    VALUES (9001, foo, 1, 2, 3, true, epoch + 0 seconds, bar, 4, 5);
  )"));
}

TEST(MockPqxxClient, RegisterBlackBoxLineage) {
  using Client = MockPqxxClient<Hash, MockToSql, MockClock>;
  ConnectionConfig c;
  StatusOr<Client> client_or = Client::Make("name", 9001, "127.0.0.1", c);
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK,
            client.RegisterBlackBoxLineage("bar", std::vector<std::string>{}));
  ASSERT_EQ(Status::OK, client.RegisterBlackBoxLineage(
                            "baz", std::vector<std::string>{"query1"}));
  ASSERT_EQ(Status::OK,
            client.RegisterBlackBoxLineage(
                "zardoz", std::vector<std::string>{"query2", "query3"}));
  std::vector<std::pair<std::string, std::string>> queries = client.Queries();

  ASSERT_EQ(queries.size(), static_cast<std::size_t>(8));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, R"(
    UPDATE Collections
    SET lineage_type = 'sql'
    WHERE node_id = 9001 AND collection_name = bar;
  )");
  ExpectStringsEqualIgnoreWhiteSpace(queries[3].second, R"(
    UPDATE Collections
    SET lineage_type = 'sql'
    WHERE node_id = 9001 AND collection_name = baz;
  )");
  ExpectStringsEqualIgnoreWhiteSpace(queries[4].second, "query1");
  ExpectStringsEqualIgnoreWhiteSpace(queries[5].second, R"(
    UPDATE Collections
    SET lineage_type = 'sql'
    WHERE node_id = 9001 AND collection_name = zardoz;
  )");
  ExpectStringsEqualIgnoreWhiteSpace(queries[6].second, "query2");
  ExpectStringsEqualIgnoreWhiteSpace(queries[7].second, "query3");
}

TEST(MockPqxxClient, RegisterBlackBoxPythonLineageScript) {
  using Client = MockPqxxClient<Hash, MockToSql, MockClock>;
  ConnectionConfig c;
  StatusOr<Client> client_or = Client::Make("name", 9001, "127.0.0.1", c);
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK,
            client.RegisterBlackBoxPythonLineageScript("rick\nand\nmorty"));
  ASSERT_EQ(Status::OK, client.RegisterBlackBoxPythonLineageScript(
                            "beevis\nand\nbutthead"));
  std::vector<std::pair<std::string, std::string>> queries = client.Queries();

  ASSERT_EQ(queries.size(), static_cast<std::size_t>(4));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second,
                                     fmt::format(R"(
    UPDATE Nodes
    SET python_lineage_script = E{}
    WHERE node_id = 9001;
  )",
                                                 "rick\nand\nmorty"));
  ExpectStringsEqualIgnoreWhiteSpace(queries[3].second,
                                     fmt::format(R"(
    UPDATE Nodes
    SET python_lineage_script = E{}
    WHERE node_id = 9001;
  )",
                                                 "beevis\nand\nbutthead"));
}

TEST(MockPqxxClient, RegisterBlackBoxPythonLineage) {
  using Client = MockPqxxClient<Hash, MockToSql, MockClock>;
  ConnectionConfig c;
  StatusOr<Client> client_or = Client::Make("name", 9001, "127.0.0.1", c);
  ASSERT_EQ(Status::OK, client_or.status());
  Client client = client_or.ConsumeValueOrDie();
  ASSERT_EQ(Status::OK, client.RegisterBlackBoxPythonLineage("foo", "get"));
  ASSERT_EQ(Status::OK, client.RegisterBlackBoxPythonLineage("bar", "set"));
  std::vector<std::pair<std::string, std::string>> queries = client.Queries();

  ASSERT_EQ(queries.size(), static_cast<std::size_t>(4));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, R"(
    UPDATE Collections
    SET python_lineage_method = get
    WHERE node_id = 9001 AND collection_name = foo;
  )");
  ExpectStringsEqualIgnoreWhiteSpace(queries[3].second, R"(
    UPDATE Collections
    SET python_lineage_method = set
    WHERE node_id = 9001 AND collection_name = bar;
  )");
}

}  // namespace lineagedb
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
