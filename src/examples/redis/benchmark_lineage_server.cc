#include <cstdint>
#include <string>

#include "fmt/format.h"
#include "glog/logging.h"
#include "redox.hpp"
#include "zmq.hpp"

#include "common/mock_pickler.h"
#include "common/status.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/async_pqxx_client.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/pqxx_client.h"
#include "ra/logical/all.h"

namespace lra = fluent::ra::logical;
namespace ldb = fluent::lineagedb;

using fluent::common::Hash;
using fluent::common::MockPickler;
using fluent::lineagedb::AsyncPqxxClient;
using fluent::lineagedb::ToSql;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 9) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_host> \\" << std::endl               //
              << "  <db_port> \\" << std::endl               //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <redis_addr> \\" << std::endl            //
              << "  <redis_port> \\" << std::endl            //
              << "  <address> \\" << std::endl               //
        ;
    return 1;
  }

  const std::string db_host = argv[1];
  const int db_port = std::stoi(argv[2]);
  const std::string db_user = argv[3];
  const std::string db_password = argv[4];
  const std::string db_dbname = argv[5];
  const std::string redis_addr = argv[6];
  const int redis_port = std::stoi(argv[7]);
  const std::string address = argv[8];

  redox::Redox rdx;
  rdx.noWait(true);
  CHECK(rdx.connect(redis_addr, redis_port))
      << "Could not connect to redis server listening on " << redis_addr << ":"
      << redis_port;

  ldb::ConnectionConfig conf;
  conf.host = db_host;
  conf.port = db_port;
  conf.user = db_user;
  conf.password = db_password;
  conf.dbname = db_dbname;

  zmq::context_t context(1);
  auto f =
      fluent::fluent<AsyncPqxxClient, Hash, ToSql, MockPickler>(
          "redis_benchmark_lineage_server", address, &context, conf)
          .ConsumeValueOrDie()
          .channel<std::string, std::string, std::int64_t, std::string,
                   std::string>(
              "set_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
          .channel<std::string, std::int64_t, bool>(  //
              "set_response", {{"addr", "id", "success"}})
          .RegisterRules([&](auto& set_req, auto& set_resp) {
            using namespace fluent::infix;
            auto set =
                set_resp <=
                (lra::make_collection(&set_req) |
                 lra::map([&rdx](const auto& t) {
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t id = std::get<2>(t);
                   const std::string& key = std::get<3>(t);
                   const std::string& value = std::get<4>(t);
                   return std::make_tuple(src_addr, id, rdx.set(key, value));
                 }));
            return std::make_tuple(set);
          })
          .ConsumeValueOrDie();
  CHECK_EQ(fluent::common::Status::OK, f.Run());
}
