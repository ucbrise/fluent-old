#include <cstdint>

#include <map>
#include <vector>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/rand_util.h"
#include "common/string_util.h"
#include "examples/cassandra/api.h"
#include "fluent/fluent.h"

namespace lra = fluent::ra::logical;
namespace ldb = fluent::lineagedb;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 8) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_host> \\" << std::endl               //
              << "  <db_port> \\" << std::endl               //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <server_address> \\" << std::endl        //
              << "  <client_address> \\" << std::endl;
    return 1;
  }

  // Command line arguments.
  const std::string db_host = argv[1];
  const int db_port = std::stoi(argv[2]);
  const std::string db_user = argv[3];
  const std::string db_password = argv[4];
  const std::string db_dbname = argv[5];
  const std::string server_address = argv[6];
  const std::string client_address = argv[7];

  // Random id generator.
  fluent::RandomIdGenerator id_gen;

  // ZeroMQ context.
  zmq::context_t context(1);

  // Lineage database configuration.
  ldb::ConnectionConfig config;
  config.host = db_host;
  config.port = db_port;
  config.user = db_user;
  config.password = db_password;
  config.dbname = db_dbname;

  const std::string name = "cassandra_client_" + fluent::RandomAlphanum(10);
  auto fb =
      fluent::fluent<ldb::PqxxClient>(name, client_address, &context, config)
          .ConsumeValueOrDie()
          .stdin()
          .stdout()
          .scratch<std::vector<std::string>>("split", {{"parts"}});
  fluent::Status status =
      AddKvsApi(std::move(fb))
          .RegisterRules([&](auto& stdin, auto& stdout, auto& split,
                             auto& set_request, auto& set_response,
                             auto& get_request, auto& get_response) {
            using namespace fluent::infix;

            auto buffer_stdin =
                split <= (lra::make_collection(&stdin) |
                          lra::map([](const std::tuple<std::string>& s) {
                            const std::string& line = std::get<0>(s);
                            std::vector<std::string> parts =
                                fluent::Split(line);
                            return std::tuple<std::vector<std::string>>(parts);
                          }));

            using parts_tuple = std::tuple<std::vector<std::string>>;

            auto send_get =
                get_request <=
                (lra::make_collection(&split) |
                 lra::filter([](const parts_tuple& parts_tuple) {
                   const std::vector<std::string>& parts =
                       std::get<0>(parts_tuple);
                   return parts.size() == 2 && parts[0] == "GET";
                 }) |
                 lra::map(
                     [&](const parts_tuple& parts_tuple) -> get_request_tuple {
                       const std::vector<std::string>& parts =
                           std::get<0>(parts_tuple);
                       const std::string& key = parts[1];
                       const std::int64_t id = id_gen.Generate();
                       return {server_address, client_address, id, key};
                     }));

            auto send_set =
                set_request <=
                (lra::make_collection(&split) |
                 lra::filter([](const parts_tuple& parts_tuple) {
                   const std::vector<std::string>& parts =
                       std::get<0>(parts_tuple);
                   return parts.size() == 3 && parts[0] == "SET";
                 }) |
                 lra::map(
                     [&](const parts_tuple& parts_tuple) -> set_request_tuple {
                       const std::vector<std::string>& parts =
                           std::get<0>(parts_tuple);
                       const std::string key = parts[1];
                       const std::int32_t value = std::stol(parts[2]);
                       const std::int64_t id = id_gen.Generate();
                       return {server_address, client_address, id, key, value};
                     }));

            auto print_get =
                stdout <=
                (lra::make_collection(&get_response) | lra::project<2, 3>() |
                 lra::map([](const std::tuple<std::int32_t, std::int64_t>& t) {
                   const std::string value =
                       "value = " + std::to_string(std::get<0>(t));
                   const std::string id =
                       "id = " + std::to_string(std::get<1>(t));
                   return std::tuple<std::string>(value + "\n" + id);
                 }));

            auto print_set = stdout <= (lra::make_collection(&set_response) |
                                        lra::map([](const auto&) {
                                          return std::tuple<std::string>("OK");
                                        }));

            return std::make_tuple(buffer_stdin, send_get, send_set, print_get,
                                   print_set);
          })
          .ConsumeValueOrDie()
          .Run();
  CHECK_EQ(status, fluent::Status::OK);
}
