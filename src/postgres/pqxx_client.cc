#include "postgres/pqxx_client.h"

#include "fmt/format.h"
#include "glog/logging.h"

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
  id_ = static_cast<std::int64_t>(std::hash<std::string>()(name));
  ExecuteQuery("Init", fmt::format("INSERT INTO {} ({}, {}) VALUES ({}, '{}')",
                                   NODES, NODES_ID, NODES_NAME, id_, name));
}

void PqxxClient::AddCollection(const std::string& collection_name) {
  CHECK_NE(id_, static_cast<std::int64_t>(0)) << "Call Init first.";
  ExecuteQuery("AddCollection",
               fmt::format("INSERT INTO {} ({}, {}) VALUES ({}, '{}')",
                           COLLECTIONS, COLLECTIONS_NODE_ID,
                           COLLECTIONS_COLLECTION_NAME, id_, collection_name));
}

void PqxxClient::AddRule(std::size_t rule_number, const std::string& rule) {
  CHECK_NE(id_, static_cast<std::int64_t>(0)) << "Call Init first.";
  ExecuteQuery("AddRule",
               fmt::format("INSERT INTO {} ({}, {}, {}) VALUES ({}, {}, '{}')",
                           RULES, RULES_NODE_ID, RULES_RULE_NUMBER, RULES_RULE,
                           id_, rule_number, rule));
}

}  // namespace postgres
}  // namespace fluent
