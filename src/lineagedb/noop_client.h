#ifndef LINEAGEDB_NOOP_CLIENT_H_
#define LINEAGEDB_NOOP_CLIENT_H_

#include <cstddef>

#include <array>
#include <chrono>
#include <string>

#include "common/macros.h"
#include "common/status.h"
#include "common/status_or.h"
#include "fluent/local_tuple_id.h"
#include "lineagedb/connection_config.h"

namespace fluent {
namespace lineagedb {

// A NoopClient has the same interface as a PqxxClient, but it does, well,
// nothing at all. A NoopClient can be used in place of a PqxxClient when you
// don't really want to connect to a lineagedb database and don't really care
// about history or lineage. For example, it is useful in unit tests.
template <template <typename> class Hash, template <typename> class ToSql,
          typename Clock>
class NoopClient {
 public:
  DISALLOW_COPY_AND_ASSIGN(NoopClient);
  NoopClient(NoopClient&&) = default;
  NoopClient& operator=(NoopClient&&) = default;

  static WARN_UNUSED StatusOr<NoopClient> WARN_UNUSED
  Make(std::string, std::size_t, std::string, const ConnectionConfig&) {
    return NoopClient();
  }

  template <typename... Ts>
  WARN_UNUSED Status
  AddCollection(const std::string&, const std::string&,
                const std::array<std::string, sizeof...(Ts)>&) {
    return Status::OK;
  }

  WARN_UNUSED Status AddRule(std::size_t, bool, const std::string&) {
    return Status::OK;
  }

  template <typename... Ts>
  WARN_UNUSED Status InsertTuple(const std::string&, int,
                                 const std::chrono::time_point<Clock>&,
                                 const std::tuple<Ts...>&) {
    return Status::OK;
  }

  template <typename... Ts>
  WARN_UNUSED Status DeleteTuple(const std::string&, int,
                                 const std::chrono::time_point<Clock>&,
                                 const std::tuple<Ts...>&) {
    return Status::OK;
  }

  WARN_UNUSED Status AddNetworkedLineage(std::size_t, int, const std::string&,
                                         std::size_t, int) {
    return Status::OK;
  }

  WARN_UNUSED Status AddDerivedLineage(const LocalTupleId&, int, bool,
                                       const std::chrono::time_point<Clock>&,
                                       const LocalTupleId&) {
    return Status::OK;
  }

  WARN_UNUSED Status RegisterBlackBoxLineage(const std::string&,
                                             const std::vector<std::string>&) {
    return Status::OK;
  }

 private:
  NoopClient() = default;
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_NOOP_CLIENT_H_
