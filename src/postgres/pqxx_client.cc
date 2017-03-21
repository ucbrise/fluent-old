#include "postgres/pqxx_client.h"

#include "fmt/format.h"
#include "glog/logging.h"

#include "common/string_util.h"
#include "postgres/constants.h"

namespace fluent {
namespace postgres {

PqxxClient::PqxxClient(const ConnectionConfig& connection_config)
    : connection_(connection_config.ToString()) {
  LOG(INFO) << "Established a postgres connection with the following "
               "parameters: " +
                   connection_config.ToString();
}

void PqxxClient::ExecuteQuery(const std::string& name,
                              const std::string& query) {
  pqxx::work txn(connection_, name);
  LOG(INFO) << "Executing query: " << query;
  txn.exec(query);
  txn.commit();
}

void PqxxClient::Init(const std::string& name) {
  // TODO(mwhittaker): Handle the scenario where there is a hash collision.
  name_ = name;
  id_ = static_cast<std::int64_t>(std::hash<std::string>()(name));
  ExecuteQuery("Init", fmt::format(R"(
    INSERT INTO {} ({}, {})
    VALUES ({}, '{}');)",
                                   NODES, NODES_ID, NODES_NAME, id_, name));

  ExecuteQuery("CreateLineageTable", fmt::format(R"(
    CREATE TABLE {}_lineage (
      dep_node_id          bigint   NOT NULL,
      dep_collection_name  text     NOT NULL,
      dep_tuple_hash       bigint   NOT NULL,
      rule_number          integer  NOT NULL,
      inserted             boolean  NOT NULL,
      collection_name      text     NOT NULL,
      tuple_hash           bigint   NOT NULL,
      time                 integer  NOT NULL
    );)",
                                                 name));
}

void PqxxClient::AddCollection(const std::string& collection_name,
                               const std::vector<std::string>& types) {
  CHECK_GT(types.size(), 0ul) << "Collections should have at least one column.";
  CHECK_NE(id_, static_cast<std::int64_t>(0)) << "Call Init first.";
  CHECK_NE(collection_name, std::string("lineage"))
      << "Lineage is a reserved collection name.";

  ExecuteQuery("AddCollection",
               fmt::format(R"(
    INSERT INTO {} ({}, {})
    VALUES ({}, '{}');)",
                           COLLECTIONS, COLLECTIONS_NODE_ID,
                           COLLECTIONS_COLLECTION_NAME, id_, collection_name));

  std::vector<std::string> columns;
  for (std::size_t i = 0; i < types.size(); ++i) {
    columns.push_back(fmt::format("col_{} {} NOT NULL", i, types[i]));
  }
  ExecuteQuery("AddCollectionTable",
               fmt::format(R"(
    CREATE TABLE {}_{} (
      hash          bigint  NOT NULL,
      time_inserted integer NOT NULL,
      time_deleted  integer,
      {},
      PRIMARY KEY (hash, time_inserted)
    );)",
                           name_, collection_name, Join(columns)));
}

void PqxxClient::InsertTuple(const std::string& collection_name,
                             std::size_t hash, int time_inserted,
                             const std::vector<std::string>& values) {
  CHECK_GT(values.size(), static_cast<std::size_t>(0))
      << "Collections should have at least one column.";

  ExecuteQuery("InsertTuple", fmt::format(R"(
    INSERT INTO {}_{}
    VALUES ({}, {}, NULL, {});
  )",
                                          name_, collection_name,
                                          static_cast<std::int64_t>(hash),
                                          time_inserted, Join(values)));
}

void PqxxClient::DeleteTuple(const std::string& collection_name,
                             std::size_t hash, int time_deleted) {
  ExecuteQuery("AddRule", fmt::format(R"(
    UPDATE {}
    SET time_deleted = {}
    WHERE hash={} AND time_deleted IS NULL;)",
                                      collection_name, time_deleted,
                                      static_cast<std::int64_t>(hash)));
}

void PqxxClient::AddRule(std::size_t rule_number, const std::string& rule) {
  CHECK_NE(id_, static_cast<std::int64_t>(0)) << "Call Init first.";
  ExecuteQuery("AddRule", fmt::format(R"(
    INSERT INTO {} ({}, {}, {})
    VALUES ({}, {}, '{}');)",
                                      RULES, RULES_NODE_ID, RULES_RULE_NUMBER,
                                      RULES_RULE, id_, rule_number, rule));
}

}  // namespace postgres
}  // namespace fluent
