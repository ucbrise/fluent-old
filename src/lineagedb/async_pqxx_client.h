#ifndef LINEAGEDB_ASYNC_PQXX_CLIENT_H_
#define LINEAGEDB_ASYNC_PQXX_CLIENT_H_

#include <cstdint>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "fmt/format.h"
#include "glog/logging.h"
#include "pqxx/pqxx"

#include "common/concurrent_queue.h"
#include "common/macros.h"
#include "common/status.h"
#include "common/status_macros.h"
#include "common/status_or.h"
#include "common/string_util.h"
#include "common/tuple_util.h"
#include "common/type_list.h"
#include "fluent/local_tuple_id.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/pqxx_client.h"
#include "lineagedb/to_sql.h"

namespace fluent {
namespace lineagedb {

template <template <typename> class Hash, template <typename> class ToSql,
          typename Clock>
class AsyncPqxxClient : public PqxxClient<Hash, ToSql, Clock> {
 public:
  AsyncPqxxClient() {}
  DISALLOW_COPY_AND_ASSIGN(AsyncPqxxClient);
  DISALLOW_MOVE_AND_ASSIGN(AsyncPqxxClient);
  virtual ~AsyncPqxxClient() = default;

  static WARN_UNUSED common::StatusOr<std::unique_ptr<AsyncPqxxClient>> Make(
      std::string name, std::size_t id, std::string address,
      const ConnectionConfig& connection_config) {
    try {
      std::unique_ptr<AsyncPqxxClient> client(new AsyncPqxxClient(
          std::move(name), id, std::move(address), connection_config));
      RETURN_IF_ERROR(client->Init());
      return std::move(client);
    } catch (const pqxx::pqxx_exception& e) {
      return common::Status(common::ErrorCode::INVALID_ARGUMENT,
                            e.base().what());
    }
  }

 protected:
  AsyncPqxxClient(std::string name, std::size_t id, std::string address,
                  const ConnectionConfig& connection_config)
      : PqxxClient<Hash, ToSql, Clock>(std::move(name), id, std::move(address),
                                       connection_config) {
    t_ = std::thread(&AsyncPqxxClient::ExecuteQueries, this);
  }

  WARN_UNUSED common::Status ExecuteQuery(const std::string& name,
                                          const std::string& query) override {
    queries_.Push({name, query});
    return common::Status::OK;
  }

 private:
  void ExecuteQueries() {
    while (true) {
      std::pair<std::string, std::string> name_query = queries_.Pop();
      const std::string& name = name_query.first;
      const std::string& query = name_query.second;

      try {
        pqxx::work txn(this->GetConnection(), name);
        VLOG(1) << "Executing query: " << query;
        txn.exec(query);
        txn.commit();
      } catch (const pqxx::pqxx_exception& e) {
        LOG(FATAL) << e.base().what();
      }
    }
  }

  common::ConcurrentQueue<std::pair<std::string, std::string>> queries_;
  std::thread t_;
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_ASYNC_PQXX_CLIENT_H_
