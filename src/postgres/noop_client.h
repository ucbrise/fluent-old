#ifndef POSTGRES_NOOP_CLIENT_H_
#define POSTGRES_NOOP_CLIENT_H_

#include "postgres/connection_config.h"

namespace fluent {
namespace postgres {

// DO_NOT_SUBMIT(mwhittaker): Document.
template <template <typename> class Hash, template <typename> class ToSql>
class NoopClient {
 public:
  NoopClient(const ConnectionConfig&) {}

  void Init(const std::string&) {}

  template <typename... Ts>
  void AddCollection(const std::string&) {}

  template <typename RA>
  void AddRule(std::size_t, const RA&) {}

  template <typename... Ts>
  void InsertTuple(const std::string&, int, const std::tuple<Ts...>&) {}

  template <typename... Ts>
  void DeleteTuple(const std::string&, int, const std::tuple<Ts...>&) {}
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_NOOP_CLIENT_H_
