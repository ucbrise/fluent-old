#include <iostream>

#include "glog/logging.h"

#include "examples/chat/server.h"
#include "postgres/connection_config.h"
#include "postgres/pqxx_client.h"

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 5) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <server address> \\" << std::endl;
    return 1;
  }

  ServerArgs args{argv[1]};
  fluent::postgres::ConnectionConfig config{"localhost", 5432, argv[1], argv[2],
                                            argv[3]};
  fluent::postgres::PqxxClient client(config);
  return ServerMain(args, &client);
}
