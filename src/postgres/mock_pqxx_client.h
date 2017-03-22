#ifndef POSTGRES_MOCK_PQXX_CLIENT_H_
#define POSTGRES_MOCK_PQXX_CLIENT_H_

#include <cstdint>

#include <string>
#include <utility>
#include <vector>

#include "postgres/mock_connection.h"
#include "postgres/mock_work.h"
#include "postgres/pqxx_client.h"

namespace fluent {
namespace postgres {

// DO_NOT_SUBMIT(mwhittaker): Document.
template <template <typename> class Hash, template <typename> class ToSql>
class MockPqxxClient
    : public InjectablePqxxClient<MockConnection, MockWork, Hash, ToSql> {
 public:
  MockPqxxClient(const ConnectionConfig& connection_config)
      : InjectablePqxxClient<MockConnection, MockWork, Hash, ToSql>(
            connection_config) {}

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

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_MOCK_PQXX_CLIENT_H_
