#include <iostream>

#include "glog/logging.h"

#include "examples/chat/ping_client.h"
#include "postgres/connection_config.h"
#include "postgres/noop_client.h"

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 5) {
    std::cerr << "usage: " << argv[0]
              << " <server address> <client address> <nickname> <msg>"
              << std::endl;
    return 1;
  }

  PingClientArgs args{argv[1], argv[2], argv[3], argv[4]};
  fluent::postgres::ConnectionConfig config;
  fluent::postgres::NoopClient client(config);
  return PingClientMain(args, &client);
}
