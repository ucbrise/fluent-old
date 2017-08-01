#include <cstdint>

#include <map>
#include <vector>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/rand_util.h"
#include "common/status.h"
#include "common/string_util.h"
#include "examples/redis/redis.h"
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
              << "  <server_addr> \\" << std::endl           //
              << "  <client_addr> \\" << std::endl           //
              << "  <name> \\" << std::endl                  //
        ;
    return 1;
  }

  const std::string db_user = argv[1];
  const std::string db_password = argv[2];
  const std::string db_dbname = argv[3];
  const std::string server_addr = argv[4];
  const std::string client_addr = argv[5];
  const std::string name_suffix = argv[6];

  fluent::common::RandomIdGenerator id_gen;
  const std::string name = "redis_client_" + name_suffix;
  zmq::context_t context(1);
  ConnectionConfig config;
  config.host = "localhost";
  config.port = 5432;
  config.user = db_user;
  config.password = db_password;
  config.dbname = db_dbname;

  auto fb_or = fluent::fluent<PqxxClient>(name, client_addr, &context, config);
  auto fb = fb_or.ConsumeValueOrDie();

  auto pre_collections =
      std::move(fb)  //
          .stdin()   //
          .stdout()  //
          .scratch<std::vector<std::string>>("split", {{"parts"}});
  auto collections = AddRedisApi(std::move(pre_collections));

  auto f_or =
      std::move(collections)
          .RegisterRules([&](auto& stdin, auto& stdout, auto& split,  //
                             auto& set_req, auto& set_resp,           //
                             auto& del_req, auto& del_resp,           //
                             auto& append_req, auto& append_resp,     //
                             auto& incr_req, auto& incr_resp,         //
                             auto& decr_req, auto& decr_resp,         //
                             auto& incrby_req, auto& incrby_resp,     //
                             auto& decrby_req, auto& decrby_resp,     //
                             auto& get_req, auto& get_resp,           //
                             auto& strlen_req, auto& strlen_resp) {
            using namespace fluent::infix;
            using split_tuple = std::tuple<std::vector<std::string>>;
            using stdin_tuple = std::tuple<std::string>;
            using stdout_tuple = std::tuple<std::string>;

            auto buffer_stdin =
                split <= (lra::make_collection(&stdin) |
                          lra::map([](const stdin_tuple& t) -> split_tuple {
                            const std::string& s = std::get<0>(t);
                            return std::make_tuple(fluent::common::Split(s));
                          }));

            // Requests.
            auto string_request = [&](auto& channel, const std::string& cmd) {
              return channel <=
                     (lra::make_collection(&split) |
                      lra::filter([cmd](const split_tuple& t) {
                        const std::vector<std::string>& parts = std::get<0>(t);
                        return parts.size() == 2 &&
                               fluent::common::ToLower(parts[0]) ==
                                   fluent::common::ToLower(cmd);
                      }) |
                      lra::map([&](const split_tuple& t) {
                        const std::vector<std::string>& parts = std::get<0>(t);
                        const std::int64_t id = id_gen.Generate();
                        const std::string& s = parts[1];
                        return std::make_tuple(server_addr, client_addr, id, s);
                      }));
            };

            auto string_string_request = [&](auto& channel,
                                             const std::string& cmd) {
              return channel <=
                     (lra::make_collection(&split) |
                      lra::filter([cmd](const split_tuple& t) {
                        const std::vector<std::string>& parts = std::get<0>(t);
                        return parts.size() == 3 &&
                               fluent::common::ToLower(parts[0]) ==
                                   fluent::common::ToLower(cmd);
                      }) |
                      lra::map([&](const split_tuple& t) {
                        const std::vector<std::string>& parts = std::get<0>(t);
                        const std::int64_t id = id_gen.Generate();
                        const std::string& s1 = parts[1];
                        const std::string& s2 = parts[2];
                        return std::make_tuple(server_addr, client_addr, id, s1,
                                               s2);
                      }));
            };

            auto string_int_request = [&](auto& channel,
                                          const std::string& cmd) {
              return channel <=
                     (lra::make_collection(&split) |
                      lra::filter([cmd](const split_tuple& t) {
                        const std::vector<std::string>& parts = std::get<0>(t);
                        return parts.size() == 3 &&
                               fluent::common::ToLower(parts[0]) ==
                                   fluent::common::ToLower(cmd);
                      }) |
                      lra::map([&](const split_tuple& t) {
                        const std::vector<std::string>& parts = std::get<0>(t);
                        const std::int64_t id = id_gen.Generate();
                        const std::string& s = parts[1];
                        const int i = std::stoi(parts[2]);
                        return std::make_tuple(server_addr, client_addr, id, s,
                                               i);
                      }));
            };

            auto set_request = string_string_request(set_req, "SET");
            auto del_request = string_request(del_req, "DEL");
            auto append_request = string_string_request(append_req, "APPEND");
            auto incr_request = string_request(incr_req, "INCR");
            auto decr_request = string_request(decr_req, "DECR");
            auto incrby_request = string_int_request(incrby_req, "INCRBY");
            auto decrby_request = string_int_request(decrby_req, "DECRBY");
            auto get_request = string_request(get_req, "GET");
            auto strlen_request = string_request(strlen_req, "STRLEN");

            // Reponses.
            auto string_response = [&stdout](auto& channel) {
              return stdout <= (lra::make_collection(&channel) |
                                lra::map([&](const auto& t) -> stdout_tuple {
                                  return std::make_tuple(std::get<2>(t));
                                }));
            };

            auto int_response = [&stdout](auto& channel) {
              return stdout <=
                     (lra::make_collection(&channel) |
                      lra::map([&](const auto& t) -> stdout_tuple {
                        return std::make_tuple(std::to_string(std::get<2>(t)));
                      }));
            };

            auto set_response = string_response(set_resp);
            auto del_response = int_response(del_resp);
            auto append_response = int_response(append_resp);
            auto incr_response = int_response(incr_resp);
            auto decr_response = int_response(decr_resp);
            auto incrby_response = int_response(incrby_resp);
            auto decrby_response = int_response(decrby_resp);
            auto get_response = string_response(get_resp);
            auto strlen_response = int_response(strlen_resp);

            return std::make_tuple(buffer_stdin,                     //
                                   set_request, set_response,        //
                                   del_request, del_response,        //
                                   append_request, append_response,  //
                                   incr_request, incr_response,      //
                                   decr_request, decr_response,      //
                                   incrby_request, incrby_response,  //
                                   decrby_request, decrby_response,  //
                                   get_request, get_response,        //
                                   strlen_request, strlen_response);
          });
  auto f = f_or.ConsumeValueOrDie();

  CHECK_EQ(fluent::common::Status::OK, f.Run());
}
