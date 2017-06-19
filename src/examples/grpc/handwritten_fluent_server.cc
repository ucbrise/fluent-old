#include <cstdint>

#include <string>

#include "glog/logging.h"
#include "grpc++/grpc++.h"
#include "zmq.hpp"

#include "common/status.h"
#include "examples/grpc/handwritten_api.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/pqxx_client.h"

namespace ldb = fluent::lineagedb;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 6) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <echo_address> \\" << std::endl          //
              << "  <address>" << std::endl;
    return 1;
  }

  const std::string db_user = argv[1];
  const std::string db_pass = argv[2];
  const std::string db_dbname = argv[3];
  const std::string echo_address = argv[4];
  const std::string address = argv[5];

  EchoServiceClient client(
      grpc::CreateChannel(echo_address, grpc::InsecureChannelCredentials()));

  zmq::context_t context(1);
  ldb::ConnectionConfig config{"localhost", 5432, db_user, db_pass, db_dbname};
  auto fb_or = fluent::fluent<ldb::PqxxClient>("handwritten_fluent_server",
                                               address, &context, config);
  auto fb = fb_or.ConsumeValueOrDie();
  auto f = GetEchoServiceShim(std::move(fb), &client).ConsumeValueOrDie();
  CHECK_EQ(fluent::Status::OK, f.Run());
}
