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
#include "lineagedb/connection_config.h"
#include "lineagedb/noop_client.h"
#include "ra/logical/all.h"

namespace lra = fluent::ra::logical;
namespace ldb = fluent::lineagedb;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 4) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <redis_addr> \\" << std::endl            //
              << "  <redis_port> \\" << std::endl            //
              << "  <address> \\" << std::endl               //
        ;
    return 1;
  }

  const std::string redis_addr = argv[1];
  const int redis_port = std::stoi(argv[2]);
  const std::string address = argv[3];

  redox::Redox rdx;
  rdx.noWait(true);
  CHECK(rdx.connect(redis_addr, redis_port))
      << "Could not connect to redis server listening on " << redis_addr << ":"
      << redis_port;

  zmq::context_t context(1);
  fluent::lineagedb::ConnectionConfig config;
  auto f =
      fluent::fluent<ldb::NoopClient, fluent::Hash, ldb::ToSql,
                     fluent::MockPickler>("redis_server_benchmark", address,
                                          &context, config)
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
  CHECK_EQ(fluent::Status::OK, f.Run());
}
