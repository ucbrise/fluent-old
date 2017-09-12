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
using fluent::LocalTupleId;
using fluent::common::Status;

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

  auto with_collections =
      AddRedisApi(std::move(fb))
          .table<std::string, std::size_t, int, std::string>(
              "meta", {{"c", "h", "t", "key"}});

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
                             auto& strlen_req, auto& strlen_resp,  //
                             auto& meta) {
            using namespace fluent::infix;

            // Regular rules.
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

            // Meta rules.
            auto make_meta_rule = [&](const auto& c) {
              return meta <=
                     (lra::make_meta_collection(&c) |
                      lra::map([](const auto& p) {
                        const auto& t = std::get<0>(p);
                        const std::string& key = std::get<3>(t);
                        const LocalTupleId& ltid = std::get<1>(p);
                        return std::make_tuple(ltid.collection_name, ltid.hash,
                                               ltid.logical_time_inserted, key);
                      }));
            };
            auto set_meta = make_meta_rule(set_req);
            auto del_meta = make_meta_rule(del_req);
            auto append_meta = make_meta_rule(append_req);
            auto incr_meta = make_meta_rule(incr_req);
            auto decr_meta = make_meta_rule(decr_req);
            auto incrby_meta = make_meta_rule(incrby_req);
            auto decrby_meta = make_meta_rule(decrby_req);

            return std::make_tuple(set, del, append, incr, decr, incrby, decrby,
                                   get, strlen, set_meta, del_meta, append_meta,
                                   incr_meta, decr_meta, incrby_meta,
                                   decrby_meta);
          });
  auto f = f_or.ConsumeValueOrDie();

  // Register lineage for get and strlen.
  auto get_and_strlen_lineage = [](const std::string& time_inserted,
                                   const std::string& key,
                                   const std::string& ret) {
    UNUSED(ret);

    const std::string q = R"(
      -- Most recent SET time.
      WITH max_set_time AS (
        SELECT MAX(time_inserted) as max_set_time
        FROM redis_server_set_request
        WHERE key = {key} AND time_inserted <= {time_inserted}
      )

      -- All modifying operations since then.
      SELECT CAST('redis_server' as TEXT),
             c,
             h,
             t,
             physical_time_inserted
      FROM redis_server_meta, max_set_time
      WHERE key={key} AND t >= max_set_time.max_set_time;
    )";
    return fmt::format(q, fmt::arg("key", key),
                       fmt::arg("time_inserted", time_inserted));
  };
  Status status = f.RegisterBlackBoxLineage<14, 15>(get_and_strlen_lineage);
  CHECK_EQ(fluent::common::Status::OK, status);
  status = f.RegisterBlackBoxLineage<16, 17>(get_and_strlen_lineage);
  CHECK_EQ(fluent::common::Status::OK, status);

  // Run the server.
  CHECK_EQ(fluent::common::Status::OK, f.Run());
}
