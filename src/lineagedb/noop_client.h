#ifndef LINEAGEDB_NOOP_CLIENT_H_
#define LINEAGEDB_NOOP_CLIENT_H_

#include <cstddef>

#include <array>
#include <string>

#include "common/macros.h"
#include "common/status.h"
#include "common/status_or.h"
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
  static WARN_UNUSED StatusOr<NoopClient> WARN_UNUSED
  Make(std::string, std::size_t, std::string, const ConnectionConfig&) {
    return NoopClient();
  }

  Status Init() { return Status::OK; }

  template <typename... Ts>
  Status AddCollection(const std::string&, const std::string&,
                       const std::array<std::string, sizeof...(Ts)>&) {
    return Status::OK;
  }

  Status AddRule(std::size_t, bool, const std::string&) { return Status::OK; }

  template <typename... Ts>
  Status InsertTuple(const std::string&, int, const std::tuple<Ts...>&) {
    return Status::OK;
  }

  template <typename... Ts>
  Status DeleteTuple(const std::string&, int, const std::tuple<Ts...>&) {
    return Status::OK;
  }

  Status AddNetworkedLineage(std::size_t, int, const std::string&, std::size_t,
                             int) {
    return Status::OK;
  }

  Status AddDerivedLineage(const std::string&, std::size_t, int, bool,
                           const std::string&, std::size_t, int) {
    return Status::OK;
  }

  Status Exec(const std::string&) { return Status::OK; }

 private:
  NoopClient(){};
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_NOOP_CLIENT_H_
