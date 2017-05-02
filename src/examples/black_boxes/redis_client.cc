#include <cstdint>

#include <map>
#include <vector>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/status.h"
#include "common/string_util.h"
#include "examples/black_boxes/random_id_generator.h"
#include "examples/black_boxes/redis.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/pqxx_client.h"
#include "ra/all.h"

namespace ra = fluent::ra;
namespace ldb = fluent::lineagedb;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 7) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <server_address> \\" << std::endl        //
              << "  <client_address> \\" << std::endl        //
              << "  <name> \\" << std::endl                  //
        ;
    return 1;
  }

  const std::string db_user = argv[1];
  const std::string db_password = argv[2];
  const std::string db_dbname = argv[3];
  const std::string server_address = argv[4];
  const std::string client_address = argv[5];
  const std::string name = argv[6];
  RandomIdGenerator id_gen;

  zmq::context_t context(1);
  ldb::ConnectionConfig config{"localhost", 5432, db_user, db_password,
                               db_dbname};
  auto fb = fluent::fluent<ldb::PqxxClient>("redis_client_" + name,
                                            client_address, &context, config)
                .ConsumeValueOrDie()
                .stdin()
                .stdout()
                .scratch<std::vector<std::string>>("split", {{"parts"}});
  AddRedisApi(std::move(fb))
      .RegisterRules([&](auto& stdin, auto& stdout, auto& split, auto& set_req,
                         auto& set_resp, auto& get_req, auto& get_resp) {
        using namespace fluent::infix;

        (void)set_resp;

        auto buffer_stdin =
            split <= (stdin.Iterable() |
                      ra::map([](const std::tuple<std::string>& s)
                                  -> std::tuple<std::vector<std::string>> {
                        return {fluent::Split(std::get<0>(s))};
                      }));

        auto get_request =
            get_req <=
            (split.Iterable() |
             ra::filter([](
                 const std::tuple<std::vector<std::string>>& parts_tuple) {
               const std::vector<std::string>& parts = std::get<0>(parts_tuple);
               return parts.size() == 2 && parts[0] == "GET";
             }) |
             ra::map(
                 [&](const std::tuple<std::vector<std::string>>& parts_tuple)
                     -> std::tuple<std::string, std::string, std::int64_t,
                                   std::string> {
                   const std::vector<std::string>& parts =
                       std::get<0>(parts_tuple);
                   return {server_address, client_address, id_gen.Generate(),
                           parts[1]};
                 }));

        auto set_request =
            set_req <=
            (split.Iterable() |
             ra::filter([](
                 const std::tuple<std::vector<std::string>>& parts_tuple) {
               const std::vector<std::string>& parts = std::get<0>(parts_tuple);
               return parts.size() == 3 && parts[0] == "SET";
             }) |
             ra::map(
                 [&](const std::tuple<std::vector<std::string>>& parts_tuple)
                     -> std::tuple<std::string, std::string, std::int64_t,
                                   std::string, std::string> {
                   const std::vector<std::string>& parts =
                       std::get<0>(parts_tuple);
                   return {server_address, client_address, id_gen.Generate(),
                           parts[1], parts[2]};
                 }));

        auto get_response = stdout <= (get_resp.Iterable() | ra::project<2>());

        return std::make_tuple(buffer_stdin, get_request, set_request,
                               get_response);
      })
      .ConsumeValueOrDie()
      .Run();
}
