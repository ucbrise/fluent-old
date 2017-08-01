#include <cstdint>

#include <string>

#include "fmt/format.h"
#include "glog/logging.h"
#include "redox.hpp"
#include "zmq.hpp"

#include "common/status.h"
#include "examples/redis/redis.h"
#include "examples/redis/wrappers.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/pqxx_client.h"
#include "ra/logical/all.h"

namespace lra = fluent::ra::logical;

using fluent::lineagedb::ConnectionConfig;
using fluent::lineagedb::PqxxClient;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 7) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <redis_addr> \\" << std::endl            //
              << "  <redis_port> \\" << std::endl            //
              << "  <address> \\" << std::endl;
    return 1;
  }

  const std::string db_user = argv[1];
  const std::string db_password = argv[2];
  const std::string db_dbname = argv[3];
  const std::string redis_addr = argv[4];
  const int redis_port = std::stoi(argv[5]);
  const std::string address = argv[6];

  redox::Redox rdx;
  CHECK(rdx.connect(redis_addr, redis_port))
      << "Could not connect to redis server listening on " << redis_addr << ":"
      << redis_port;

  const std::string name = "redis_server";
  zmq::context_t context(1);
  ConnectionConfig config;
  config.host = "localhost";
  config.port = 5432;
  config.user = db_user;
  config.password = db_password;
  config.dbname = db_dbname;

  auto fb_or = fluent::fluent<PqxxClient>(name, address, &context, config);
  auto fb = fb_or.ConsumeValueOrDie();

  auto with_collections = AddRedisApi(std::move(fb));

  auto f_or =
      std::move(with_collections)
          .RegisterRules([&](auto& set_req, auto& set_resp,        //
                             auto& del_req, auto& del_resp,        //
                             auto& append_req, auto& append_resp,  //
                             auto& incr_req, auto& incr_resp,      //
                             auto& decr_req, auto& decr_resp,      //
                             auto& incrby_req, auto& incrby_resp,  //
                             auto& decrby_req, auto& decrby_resp,  //
                             auto& get_req, auto& get_resp,        //
                             auto& strlen_req, auto& strlen_resp) {
            using namespace fluent::infix;

            // UNUSED(set_req);
            // UNUSED(set_resp);
            // UNUSED(del_req);
            // UNUSED(del_resp);
            // UNUSED(append_req);
            // UNUSED(append_resp);
            // UNUSED(incr_req);
            // UNUSED(incr_resp);
            // UNUSED(decr_req);
            // UNUSED(decr_resp);
            // UNUSED(incrby_req);
            // UNUSED(incrby_resp);
            // UNUSED(decrby_req);
            // UNUSED(decrby_resp);
            // UNUSED(get_req);
            // UNUSED(get_resp);
            // UNUSED(strlen_req);
            // UNUSED(strlen_resp);

            auto set =
                set_resp <=
                (lra::make_collection(&set_req) |
                 lra::map([&rdx](const set_req_tuple& t) -> set_resp_tuple {
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t id = std::get<2>(t);
                   const std::string& key = std::get<3>(t);
                   const std::string& value = std::get<4>(t);

                   CommandWrapper<std::string> w{
                       rdx.commandSync<std::string>({"SET", key, value})};
                   redox::Command<std::string>& cmd = w.cmd;
                   CHECK(cmd.ok()) << cmd.cmd();
                   return std::make_tuple(src_addr, id, cmd.reply());
                 }));

            auto del =
                del_resp <=
                (lra::make_collection(&del_req) |
                 lra::map([&rdx](const del_req_tuple& t) -> del_resp_tuple {
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t id = std::get<2>(t);
                   const std::string& key = std::get<3>(t);

                   CommandWrapper<int> w{rdx.commandSync<int>({"DEL", key})};
                   redox::Command<int>& cmd = w.cmd;
                   CHECK(cmd.ok()) << cmd.cmd();
                   return std::make_tuple(src_addr, id, cmd.reply());
                 }));

            auto append =
                append_resp <=
                (lra::make_collection(&append_req) |
                 lra::map(
                     [&rdx](const append_req_tuple& t) -> append_resp_tuple {
                       const std::string& src_addr = std::get<1>(t);
                       const std::int64_t id = std::get<2>(t);
                       const std::string& key = std::get<3>(t);
                       const std::string& value = std::get<4>(t);

                       CommandWrapper<int> w{
                           rdx.commandSync<int>({"APPEND", key, value})};
                       redox::Command<int>& cmd = w.cmd;
                       CHECK(cmd.ok()) << cmd.cmd();
                       return std::make_tuple(src_addr, id, cmd.reply());
                     }));

            auto incr =
                incr_resp <=
                (lra::make_collection(&incr_req) |
                 lra::map([&rdx](const incr_req_tuple& t) -> incr_resp_tuple {
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t id = std::get<2>(t);
                   const std::string& key = std::get<3>(t);

                   CommandWrapper<int> w{rdx.commandSync<int>({"INCR", key})};
                   redox::Command<int>& cmd = w.cmd;
                   CHECK(cmd.ok()) << cmd.cmd();
                   return std::make_tuple(src_addr, id, cmd.reply());
                 }));

            auto decr =
                decr_resp <=
                (lra::make_collection(&decr_req) |
                 lra::map([&rdx](const decr_req_tuple& t) -> decr_resp_tuple {
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t id = std::get<2>(t);
                   const std::string& key = std::get<3>(t);

                   CommandWrapper<int> w{rdx.commandSync<int>({"DECR", key})};
                   redox::Command<int>& cmd = w.cmd;
                   CHECK(cmd.ok()) << cmd.cmd();
                   return std::make_tuple(src_addr, id, cmd.reply());
                 }));

            auto incrby =
                incrby_resp <=
                (lra::make_collection(&incrby_req) |
                 lra::map(
                     [&rdx](const incrby_req_tuple& t) -> incrby_resp_tuple {
                       const std::string& src_addr = std::get<1>(t);
                       const std::int64_t id = std::get<2>(t);
                       const std::string& key = std::get<3>(t);
                       const int value = std::get<4>(t);

                       CommandWrapper<int> w{rdx.commandSync<int>(
                           {"INCRBY", key, std::to_string(value)})};
                       redox::Command<int>& cmd = w.cmd;
                       CHECK(cmd.ok()) << cmd.cmd();
                       return std::make_tuple(src_addr, id, cmd.reply());
                     }));

            auto decrby =
                decrby_resp <=
                (lra::make_collection(&decrby_req) |
                 lra::map(
                     [&rdx](const decrby_req_tuple& t) -> decrby_resp_tuple {
                       const std::string& src_addr = std::get<1>(t);
                       const std::int64_t id = std::get<2>(t);
                       const std::string& key = std::get<3>(t);
                       const int value = std::get<4>(t);

                       CommandWrapper<int> w{rdx.commandSync<int>(
                           {"DECRBY", key, std::to_string(value)})};
                       redox::Command<int>& cmd = w.cmd;
                       CHECK(cmd.ok()) << cmd.cmd();
                       return std::make_tuple(src_addr, id, cmd.reply());
                     }));

            auto get =
                get_resp <=
                (lra::make_collection(&get_req) |
                 lra::map([&rdx](const get_req_tuple& t) -> get_resp_tuple {
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t id = std::get<2>(t);
                   const std::string& key = std::get<3>(t);

                   CommandWrapper<std::string> w{
                       rdx.commandSync<std::string>({"GET", key})};
                   redox::Command<std::string>& cmd = w.cmd;
                   CHECK(cmd.ok()) << cmd.cmd();
                   return std::make_tuple(src_addr, id, cmd.reply());
                 }));

            auto strlen =
                strlen_resp <=
                (lra::make_collection(&strlen_req) |
                 lra::map([&rdx](
                              const strlen_req_tuple& t) -> strlen_resp_tuple {
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t id = std::get<2>(t);
                   const std::string& key = std::get<3>(t);

                   CommandWrapper<int> w{rdx.commandSync<int>({"STRLEN", key})};
                   redox::Command<int>& cmd = w.cmd;
                   CHECK(cmd.ok()) << cmd.cmd();
                   return std::make_tuple(src_addr, id, cmd.reply());
                 }));

            return std::make_tuple(set, del, append, incr, decr, incrby, decrby,
                                   get, strlen);
          });
  auto f = f_or.ConsumeValueOrDie();

#if 0
  fluent::common::Status status = f.RegisterBlackBoxLineage<4, 5>(
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
                 time_inserted,
                 physical_time_inserted
          FROM redis_server_set_request, max_set_time
          WHERE key = {} AND time_inserted = max_set_time.max_set_time
          UNION

          -- All subsequent APPENDs.
          SELECT CAST('redis_server' as TEXT),
                 CAST('append_request' as TEXT),
                 hash,
                 time_inserted,
                 physical_time_inserted
          FROM redis_server_append_request, max_set_time
          WHERE key={} AND time_inserted >= max_set_time.max_set_time;
        )",
                           key, time_inserted, key, key);
      });
#endif

  // CHECK_EQ(fluent::common::Status::OK, status);
  CHECK_EQ(fluent::common::Status::OK, f.Run());
}
