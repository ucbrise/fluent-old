#include <iostream>

#include "glog/logging.h"

#include "examples/chat/server.h"
#include "postgres/connection_config.h"
#include "postgres/noop_client.h"

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <server address>" << std::endl;
    return 1;
  }

  ServerArgs args{argv[1]};
  fluent::postgres::ConnectionConfig config;
  fluent::postgres::NoopClient client(config);
  return ServerMain(args, &client);
}
