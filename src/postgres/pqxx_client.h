#ifndef POSTGRES_PQXX_CLIENT_H_
#define POSTGRES_PQXX_CLIENT_H_

#include <cstdint>

#include "pqxx/pqxx"

#include "postgres/client.h"
#include "postgres/connection_config.h"

namespace fluent {
namespace postgres {

// DO_NOT_SUBMIT(mwhittaker): Document.
class PqxxClient : public Client {
 public:
  PqxxClient(const ConnectionConfig& connection_config);
  void Init(const std::string& name) override;
  void AddCollection(const std::string&) override;

 private:
  void ExecuteQuery(const std::string& name, const std::string& query);
  void AddRule(std::size_t rule_number, const std::string&) override;

  pqxx::connection connection_;
  std::int64_t id_;
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_PQXX_CLIENT_H_
