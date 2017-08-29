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
  DISALLOW_MOVE_AND_ASSIGN(NoopClient);

  static WARN_UNUSED common::StatusOr<std::unique_ptr<NoopClient>> Make(
      std::string, std::size_t, std::string, const ConnectionConfig&) {
    return std::unique_ptr<NoopClient>(new NoopClient());
  }

  template <typename... Ts>
  WARN_UNUSED common::Status AddCollection(
      const std::string&, const std::string&,
      const std::array<std::string, sizeof...(Ts)>&) {
    return common::Status::OK;
  }

  WARN_UNUSED common::Status AddRule(std::size_t, bool, const std::string&) {
    return common::Status::OK;
  }

  template <typename... Ts>
  WARN_UNUSED common::Status InsertTuple(const std::string&, int,
                                         const std::chrono::time_point<Clock>&,
                                         const std::tuple<Ts...>&) {
    return common::Status::OK;
  }

  template <typename... Ts>
  WARN_UNUSED common::Status DeleteTuple(const std::string&, int,
                                         const std::chrono::time_point<Clock>&,
                                         const std::tuple<Ts...>&) {
    return common::Status::OK;
  }

  WARN_UNUSED common::Status AddNetworkedLineage(std::size_t, int,
                                                 const std::string&,
                                                 std::size_t, int) {
    return common::Status::OK;
  }

  WARN_UNUSED common::Status AddDerivedLineage(
      const LocalTupleId&, int, bool, const std::chrono::time_point<Clock>&,
      const LocalTupleId&) {
    return common::Status::OK;
  }

  WARN_UNUSED common::Status RegisterBlackBoxLineage(
      const std::string&, const std::vector<std::string>&) {
    return common::Status::OK;
  }

  WARN_UNUSED common::Status RegisterBlackBoxPythonLineageScript(
      const std::string&) {
    return common::Status::OK;
  }

  WARN_UNUSED common::Status RegisterBlackBoxPythonLineage(const std::string&,
                                                           const std::string&) {
    return common::Status::OK;
  }

 private:
  NoopClient() = default;
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_NOOP_CLIENT_H_
