#include <iostream>

#include "glog/logging.h"

#include "examples/chat/server.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/noop_client.h"

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <server address>" << std::endl;
    return 1;
  }

  ServerArgs args{argv[1]};
  fluent::lineagedb::ConnectionConfig config;
  return ServerMain<fluent::lineagedb::NoopClient>(args, config);
}
