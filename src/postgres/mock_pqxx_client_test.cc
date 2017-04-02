#include "postgres/mock_pqxx_client.h"

#include <cstddef>
#include <cstdint>

#include <string>
#include <tuple>

#include "fmt/format.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/hash_util.h"
#include "postgres/to_sql.h"
#include "testing/test_util.h"

// These unit tests are all whitebox tests that you will probably have to change
// if you change the implementation of PqxxClient. They are mostly here just to
// make sure things compile and to catch silly trivial bugs.

namespace fluent {
namespace postgres {
namespace detail {

struct MockRule {
  std::string ToDebugString() const { return "yolo"; }
};

}  // namespace detail

TEST(MockPqxxClient, Init) {
  ConnectionConfig c;
  MockPqxxClient<Hash, ToSql> client("name", 9001, c);
  client.Init();

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();
  ASSERT_EQ(queries.size(), static_cast<std::size_t>(2));
  ExpectStringsEqualIgnoreWhiteSpace(queries[0].second, R"(
    INSERT INTO Nodes (id, name)
    VALUES (9001, 'name');
  )");
  ExpectStringsEqualIgnoreWhiteSpace(queries[1].second, R"(
    CREATE TABLE name_lineage (
      dep_node_id          bigint   NOT NULL,
      dep_collection_name  text     NOT NULL,
      dep_tuple_hash       bigint   NOT NULL,
      rule_number          integer  NOT NULL,
      inserted             boolean  NOT NULL,
      collection_name      text     NOT NULL,
      tuple_hash           bigint   NOT NULL,
      time                 integer  NOT NULL
    );
  )");
}

TEST(MockPqxxClient, AddCollection) {
  ConnectionConfig c;
  MockPqxxClient<Hash, ToSql> client("name", 9001, c);
  client.Init();
  client.AddCollection<int, char, bool>("t");

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();
  ASSERT_EQ(queries.size(), static_cast<std::size_t>(4));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, R"(
    INSERT INTO Collections (node_id, collection_name)
    VALUES (9001, 't');
  )");
  ExpectStringsEqualIgnoreWhiteSpace(queries[3].second, R"(
    CREATE TABLE name_t (
      hash          bigint  NOT NULL,
      time_inserted integer NOT NULL,
      time_deleted  integer,
      col_0 integer NOT NULL,
      col_1 char(1) NOT NULL,
      col_2 boolean NOT NULL,
      PRIMARY KEY (hash, time_inserted)
    );
  )");
}

TEST(MockPqxxClient, AddRule) {
  ConnectionConfig c;
  MockPqxxClient<Hash, ToSql> client("name", 9001, c);
  client.Init();
  client.AddRule(0, std::make_tuple(42, 42, detail::MockRule()));

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();
  ASSERT_EQ(queries.size(), static_cast<std::size_t>(3));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, R"(
    INSERT INTO Rules (node_id, rule_number, rule)
    VALUES (9001, 0, 'yolo');
  )");
}

TEST(MockPqxxClient, InsertTuple) {
  using tuple_t = std::tuple<int, bool, char>;
  tuple_t t = {1, true, 'a'};

  ConnectionConfig c;
  MockPqxxClient<Hash, ToSql> client("name", 9001, c);
  client.Init();
  client.InsertTuple("t", 42, t);

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();
  std::int64_t hash = detail::size_t_to_int64(Hash<tuple_t>()(t));

  ASSERT_EQ(queries.size(), static_cast<std::size_t>(3));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, fmt::format(R"(
    INSERT INTO name_t
    VALUES ({}, 42, NULL, 1, true, 'a');
  )",
                                                                    hash));
}

TEST(MockPqxxClient, DeleteTuple) {
  using tuple_t = std::tuple<int, bool, char>;
  tuple_t t = {1, true, 'a'};

  ConnectionConfig c;
  MockPqxxClient<Hash, ToSql> client("name", 9001, c);
  client.Init();
  client.DeleteTuple("t", 42, t);

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();
  std::int64_t hash = detail::size_t_to_int64(Hash<tuple_t>()(t));

  ASSERT_EQ(queries.size(), static_cast<std::size_t>(3));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, fmt::format(R"(
    UPDATE name_t
    SET time_deleted = 42
    WHERE hash={} AND time_deleted IS NULL;
  )",
                                                                    hash));
}

TEST(MockPqxxClient, AddLineage) {
  using tuple_t = std::tuple<int, bool, char>;
  tuple_t t = {1, true, 'a'};

  ConnectionConfig c;
  MockPqxxClient<Hash, ToSql> client("name", 9001, c);
  client.Init();
  client.AddLineage(0, "foo", 1, 2, true, "bar", 3, 4);

  std::vector<std::pair<std::string, std::string>> queries = client.Queries();

  ASSERT_EQ(queries.size(), static_cast<std::size_t>(3));
  ExpectStringsEqualIgnoreWhiteSpace(queries[2].second, fmt::format(R"(
    INSERT INTO name_lineage (dep_node_id, dep_collection_name, dep_tuple_hash,
                              rule_number, inserted, collection_name,
                              tuple_hash, time)
    VALUES (0, 'foo', 1, 2, true, 'bar', 3, 4);
  )"));
}

}  // namespace postgres
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
