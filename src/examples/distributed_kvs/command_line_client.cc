#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "glog/logging.h"
#include "grpc++/grpc++.h"

#include "common/rand_util.h"
#include "common/string_util.h"
#include "examples/distributed_kvs/client.h"

using ::google::protobuf::int32;
using ::google::protobuf::int64;

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <server_address>" << std::endl;
    return 1;
  }

  const std::string server_address = argv[1];
  fluent::common::RandomIdGenerator idgen;

  KeyValueServiceClient client(
      grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
  std::string line;
  std::cout << "> " << std::flush;
  while (std::getline(std::cin, line)) {
    const std::vector<std::string> parts = fluent::common::Split(line);
    if (parts.size() == 2 && parts[0] == "GET") {
      std::tuple<int32, int64> t = client.Get(parts[1]);
      std::cout << "value = " << std::get<0>(t) << std::endl
                << "id = " << std::get<1>(t) << std::endl;
    } else if (parts.size() == 3 && parts[0] == "SET") {
      int64 id = idgen.Generate();
      int64 timestamp =
          std::chrono::system_clock::now().time_since_epoch().count();
      bool b = client.Set(parts[1], std::stol(parts[2]), id, timestamp);
      std::cout << (b ? "true" : "false") << std::endl;
    } else {
      std::cout << "Error: Unrecognized command " << line << std::endl;
    }
    std::cout << "> " << std::flush;
  }
}
