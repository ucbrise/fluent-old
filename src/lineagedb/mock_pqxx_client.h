#ifndef LINEAGEDB_MOCK_PQXX_CLIENT_H_
#define LINEAGEDB_MOCK_PQXX_CLIENT_H_

#include <cstdint>

#include <string>
#include <utility>
#include <vector>

#include "lineagedb/mock_connection.h"
#include "lineagedb/mock_work.h"
#include "lineagedb/pqxx_client.h"

namespace fluent {
namespace lineagedb {

// A MockPqxxClient is like a PqxxClient, except that instead of issuing SQL
// queries, it stores them in a vector. For example:
//
//   // Create and initialize a MockPqxxClient.
//   ConnectionConfig config;
//   MockPqxxClient<Hash, ToSql> mock_client(config);
//   mock_client.Init("my_fluent_node");
//
//   // All the queries generate by `Init` are stored.
//   for (const auto& query : mock_client.Queries()) {
//     // Print the name of the query.
//     std::cout << query.first << std::endl;
//
//     // Print the SQL of the query.
//     std::cout << query.second << std::endl;
//   }
//
// A MockPqxxClient is used to unit test a PqxxClient. See
// `mock_pqxx_client_test.cc`.
template <template <typename> class Hash, template <typename> class ToSql>
class MockPqxxClient
    : public InjectablePqxxClient<MockConnection, MockWork, Hash, ToSql> {
 public:
  MockPqxxClient(std::string name, std::size_t id,
                 const ConnectionConfig& connection_config)
      : InjectablePqxxClient<MockConnection, MockWork, Hash, ToSql>(
            std::move(name), id, connection_config) {}

  void ExecuteQuery(const std::string& name,
                    const std::string& query) override {
    queries_.push_back({name, query});
  }

  const std::vector<std::pair<std::string, std::string>>& Queries() {
    return queries_;
  }

 private:
  std::vector<std::pair<std::string, std::string>> queries_;
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_MOCK_PQXX_CLIENT_H_
