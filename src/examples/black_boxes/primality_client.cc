#include <cstdint>

#include <string>

#include "glog/logging.h"
#include "zmq.hpp"

#include "common/status.h"
#include "examples/black_boxes/primality.h"
#include "examples/black_boxes/random_id_generator.h"
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

  if (argc != 6) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <server_address> \\" << std::endl        //
              << "  <client_address> \\" << std::endl;
    return 1;
  }

  const std::string server_address = argv[4];
  const std::string client_address = argv[5];
  RandomIdGenerator id_gen;

  zmq::context_t context(1);
  ldb::ConnectionConfig config{"localhost", 5432, argv[1], argv[2], argv[3]};
  auto fb = fluent::fluent<ldb::PqxxClient>("key_value_server", client_address,
                                            &context, config)
                .ConsumeValueOrDie()
                .stdin()
                .stdout();
  fluent::Status status =
      AddPrimalityApi(std::move(fb))
          .RegisterRules([&](auto& stdin, auto& stdout, auto& req, auto& resp) {
            using namespace fluent::infix;

            auto in_to_req =
                req <= (stdin.Iterable() |
                        ra::map([&](const std::tuple<std::string>& t) {
                          return std::tuple<std::string, std::string,
                                            std::int64_t, int>(
                              server_address, client_address, id_gen.Generate(),
                              std::stoi(std::get<0>(t)));
                        }));

            auto resp_to_out =
                stdout <= (resp.Iterable() | ra::project<2>() |
                           ra::map([](const std::tuple<bool>& t) {
                             return std::tuple<std::string>(
                                 std::get<0>(t) ? "prime" : "not prime");
                           }));

            return std::make_tuple(in_to_req, resp_to_out);
          })
          .ConsumeValueOrDie()
          .Run();
  CHECK_EQ(fluent::Status::OK, status);
}
