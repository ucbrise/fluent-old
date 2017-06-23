#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "glog/logging.h"
#include "grpc++/grpc++.h"

#include "common/string_util.h"
#include "examples/distributed_key_value_store/client.h"

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <server_address>" << std::endl;
    return 1;
  }

  const std::string server_address = argv[1];

  KeyValueServiceClient client(
      grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
  std::string line;
  std::cout << "> " << std::flush;
  while (std::getline(std::cin, line)) {
    const std::vector<std::string> parts = fluent::Split(line);
    if (parts.size() == 2 && parts[0] == "get") {
      std::cout << client.Get(parts[1]) << std::endl;
    } else if (parts.size() == 3 && parts[0] == "set") {
      client.Set(parts[1], std::stoi(parts[2]));
    } else {
      std::cout << "Error: Unrecognized command " << line << std::endl;
    }
    std::cout << "> " << std::flush;
  }
}
