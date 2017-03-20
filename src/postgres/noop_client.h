#ifndef POSTGRES_NOOP_CLIENT_H_
#define POSTGRES_NOOP_CLIENT_H_

#include "postgres/client.h"
#include "postgres/connection_config.h"

namespace fluent {
namespace postgres {

// DO_NOT_SUBMIT(mwhittaker): Document.
class NoopClient : public Client {
 public:
  NoopClient(const ConnectionConfig&) {}
  void Init(const std::string&) override {}
  void AddCollection(const std::string&,
                     const std::vector<std::string>&) override {}

 private:
  void AddRule(std::size_t, const std::string&) override {}
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_NOOP_CLIENT_H_
