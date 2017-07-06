#include <cstdint>
#include <string>

#include "fmt/format.h"
#include "glog/logging.h"
#include "redox.hpp"
#include "zmq.hpp"

#include "common/status.h"
#include "examples/redis/redis.h"
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
      AddRedisApi(fluent::fluent<fluent::lineagedb::NoopClient>("redis_server", address,
                                                  &context, config)
                      .ConsumeValueOrDie())
          .RegisterRules([&](auto& set_req, auto& set_resp, auto& append_req,
                             auto& append_resp, auto& get_req, auto& get_resp) {
            using namespace fluent::infix;

            (void)append_req;
            (void)append_resp;
            (void)get_req;
            (void)get_resp;

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
  fluent::Status status = f.RegisterBlackBoxLineage<4, 5>(
      [](const std::string& time_inserted, const std::string& key,
         const std::string& value) {
        (void)value;
        return fmt::format(R"(
          -- Most recent SET time.
          WITH max_set_time AS (
            SELECT MAX(time_inserted) as max_set_time
            FROM redis_server_set_request
            WHERE key = {} AND time_inserted <= {}
          )

          -- Most recent SET.
          SELECT CAST('redis_server' as TEXT),
                 CAST('set_request' as TEXT),
                 hash,
                 time_inserted
          FROM redis_server_set_request, max_set_time
          WHERE key = {} AND time_inserted = max_set_time.max_set_time
          UNION

          -- All subsequent APPENDs.
          SELECT CAST('redis_server' as TEXT),
                 CAST('append_request' as TEXT),
                 hash,
                 time_inserted
          FROM redis_server_append_request, max_set_time
          WHERE key={} AND time_inserted >= max_set_time.max_set_time;
        )",
                           key, time_inserted, key, key);
      });
  CHECK_EQ(fluent::Status::OK, status);
  CHECK_EQ(fluent::Status::OK, f.Run());
}
