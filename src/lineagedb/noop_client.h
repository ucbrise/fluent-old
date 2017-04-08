#ifndef LINEAGEDB_NOOP_CLIENT_H_
#define LINEAGEDB_NOOP_CLIENT_H_

#include "lineagedb/connection_config.h"

namespace fluent {
namespace lineagedb {

// A NoopClient has the same interface as a PqxxClient, but it does, well,
// nothing at all. A NoopClient can be used in place of a PqxxClient when you
// don't really want to connect to a lineagedb database and don't really care
// about history or lineage. For example, it is useful in unit tests.
template <template <typename> class Hash, template <typename> class ToSql>
class NoopClient {
 public:
  NoopClient(std::string, std::size_t, std::string, const ConnectionConfig&) {}

  void Init() {}

  template <typename... Ts>
  void AddCollection(const std::string&) {}

  void AddRule(std::size_t, bool, const std::string&) {}

  template <typename... Ts>
  void InsertTuple(const std::string&, int, const std::tuple<Ts...>&) {}

  template <typename... Ts>
  void DeleteTuple(const std::string&, int, const std::tuple<Ts...>&) {}

  void AddNetworkedLineage(std::size_t, int, const std::string&, std::size_t,
                           int) {}

  void AddDerivedLineage(const std::string&, std::size_t, int, bool,
                         const std::string&, std::size_t, int) {}
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_NOOP_CLIENT_H_
