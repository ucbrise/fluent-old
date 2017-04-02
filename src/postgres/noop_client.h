#ifndef POSTGRES_NOOP_CLIENT_H_
#define POSTGRES_NOOP_CLIENT_H_

#include "postgres/connection_config.h"

namespace fluent {
namespace postgres {

// A NoopClient has the same interface as a PqxxClient, but it does, well,
// nothing at all. A NoopClient can be used in place of a PqxxClient when you
// don't really want to connect to a postgres database and don't really care
// about history or lineage. For example, it is useful in unit tests.
template <template <typename> class Hash, template <typename> class ToSql>
class NoopClient {
 public:
  NoopClient(const ConnectionConfig&) {}

  void Init(const std::string&) {}

  std::size_t GetId() { return 42; }

  template <typename... Ts>
  void AddCollection(const std::string&) {}

  template <typename RA>
  void AddRule(std::size_t, const RA&) {}

  template <typename... Ts>
  void InsertTuple(const std::string&, int, const std::tuple<Ts...>&) {}

  template <typename... Ts>
  void DeleteTuple(const std::string&, int, const std::tuple<Ts...>&) {}

  void AddLineage(std::size_t, const std::string&, std::size_t, int, bool,
                  const std::string&, std::size_t, int) {}
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_NOOP_CLIENT_H_
