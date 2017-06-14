#include <cstdint>

#include <string>
#include <vector>

#include "glog/logging.h"
#include "zmq.hpp"

#include "common/random_id_generator.h"
#include "common/status.h"
#include "common/string_util.h"
#include "examples/file_system/file_system.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/pqxx_client.h"
#include "ra/logical/all.h"

namespace lra = fluent::ra::logical;
namespace ldb = fluent::lineagedb;

using split_tuple = std::tuple<std::vector<std::string>>;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 6) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <server_address> \\" << std::endl        //
              << "  <client_address> \\" << std::endl;
    return 1;
  }

  const std::string db_user = argv[1];
  const std::string db_pass = argv[2];
  const std::string db_dbname = argv[3];
  const std::string server_address = argv[4];
  const std::string client_address = argv[5];
  RandomIdGenerator id_gen;

  zmq::context_t context(1);
  ldb::ConnectionConfig config{"localhost", 5432, db_user, db_pass, db_dbname};
  auto fb = fluent::fluent<ldb::PqxxClient>("file_system_client",
                                            client_address, &context, config)
                .ConsumeValueOrDie()
                .stdin()
                .stdout()
                .scratch<std::vector<std::string>>("split", {{"parts"}});
  fluent::Status status =
      AddFileSystemApi(std::move(fb))
          .RegisterRules([&](auto& stdin, auto& stdout, auto& split,
                             auto& write_req, auto& write_resp, auto& read_req,
                             auto& read_resp) {
            using namespace fluent::infix;

            auto split_stdin =
                split <=
                (lra::make_collection(&stdin) |
                 lra::map([](const std::tuple<std::string>& s) -> split_tuple {
                   return {fluent::Split(std::get<0>(s))};
                 }));

            auto send_write_req =
                write_req <=
                (lra::make_collection(&split) |
                 lra::filter([](const split_tuple& parts_tuple) {
                   const std::vector<std::string>& parts =
                       std::get<0>(parts_tuple);
                   return parts.size() == 3 && parts[0] == "write";
                 }) |
                 lra::map(
                     [&](const split_tuple& parts_tuple) -> write_req_tuple {
                       const std::vector<std::string>& parts =
                           std::get<0>(parts_tuple);
                       const int start = std::stoi(parts[1]);
                       const std::string& data = parts[2];
                       return {server_address, client_address,
                               id_gen.Generate(), start, data};
                     }));

            auto recv_write_resp =
                stdout <= (lra::make_collection(&write_resp) |
                           lra::map([](const write_resp_tuple&) {
                             return std::tuple<std::string>("OK");
                           }));

            auto send_read_req =
                read_req <=
                (lra::make_collection(&split) |
                 lra::filter([](const split_tuple& parts_tuple) {
                   const std::vector<std::string>& parts =
                       std::get<0>(parts_tuple);
                   return parts.size() == 3 && parts[0] == "read";
                 }) |
                 lra::map(
                     [&](const split_tuple& parts_tuple) -> read_req_tuple {
                       const std::vector<std::string>& parts =
                           std::get<0>(parts_tuple);
                       const int start = std::stoi(parts[1]);
                       const int stop = std::stoi(parts[2]);
                       return {server_address, client_address,
                               id_gen.Generate(), start, stop};
                     }));

            auto recv_read_resp = stdout <= (lra::make_collection(&read_resp) |
                                             lra::project<2>());

            return std::make_tuple(split_stdin, send_write_req, recv_write_resp,
                                   send_read_req, recv_read_resp);
          })
          .ConsumeValueOrDie()
          .Run();
  CHECK_EQ(fluent::Status::OK, status);
}
