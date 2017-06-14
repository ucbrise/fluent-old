#include <iostream>

#include "glog/logging.h"
#include "zmq.hpp"

#include "zmq_util/zmq_util.h"

namespace zmq_util = fluent::zmq_util;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <address>" << std::endl;
    return 1;
  }

  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REQ);
  const std::string address = argv[1];
  socket.connect(address);
  std::cout << "Client connected to '" << address << "'." << std::endl;

  std::string line;
  std::cout << "> " << std::flush;
  while (std::getline(std::cin, line)) {
    zmq_util::send_string(line, &socket);
    std::string reply = zmq_util::recv_string(&socket);
    std::cout << reply << std::endl;
    std::cout << "> " << std::flush;
  }
}
