#include <cstdint>

#include <map>
#include <vector>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/rand_util.h"
#include "common/status.h"
#include "common/string_util.h"
#include "examples/grpc/api.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/pqxx_client.h"
#include "ra/logical/all.h"

namespace lra = fluent::ra::logical;
namespace ldb = fluent::lineagedb;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 6) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <server_address> \\" << std::endl        //
              << "  <client_address>" << std::endl;
    return 1;
  }

  const std::string db_user = argv[1];
  const std::string db_pass = argv[2];
  const std::string db_dbname = argv[3];
  const std::string server_address = argv[4];
  const std::string client_address = argv[5];
  fluent::common::RandomIdGenerator id_gen;

  zmq::context_t context(1);
  ldb::ConnectionConfig config{"localhost", 5432, db_user, db_pass, db_dbname};
  auto fb = fluent::fluent<ldb::PqxxClient>(
                "fluent_client_" + fluent::common::RandomAlphanum(10),
                client_address, &context, config)
                .ConsumeValueOrDie()
                .stdin()
                .stdout();
  fluent::common::Status status =
      AddEchoServiceApi(std::move(fb))
          .RegisterRules([&](auto& stdin, auto& stdout, auto& echo_request,
                             auto& echo_reply) {
            using namespace fluent::infix;

            auto stdin_to_echo =
                echo_request <=
                (lra::make_collection(&stdin) |
                 lra::map([&](const std::tuple<std::string>& t)
                              -> std::tuple<std::string, std::string,
                                            std::int64_t, std::string> {
                   return {server_address, client_address, id_gen.Generate(),
                           std::get<0>(t)};
                 }));

            auto echo_to_stdout = stdout <= (lra::make_collection(&echo_reply) |
                                             lra::project<2>());

            return std::make_tuple(stdin_to_echo, echo_to_stdout);
          })
          .ConsumeValueOrDie()
          .Run();
  CHECK_EQ(status, fluent::common::Status::OK);
}
