#include <cstddef>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "glog/logging.h"
#include "zmq.hpp"

#include "common/string_util.h"
#include "zmq_util/zmq_util.h"

namespace zmq_util = fluent::zmq_util;

// This file implements a simple (and I mean really simple) file server. The
// file server manages a single file which is actually just a string. Intially,
// the string is of length zero. Its API has two functions:
//
//   1. void write(start: int, bytes: string)
//   2. string read(start: int, stop: int)
//
// Writing to the string extends it with spaces. Reading from the string
// returns only the valid part of the string. Here's an example:
//
//   | command          | string  | return  |
//   | ---------------- | ------- | ------- |
//   |                  | ""      |         |
//   | write(0, "a")    | "a"     |         |
//   | write(1, "b")    | "ab"    |         |
//   | write(3, "c")    | "ab c"  |         |
//   | write(1, "defg") | "adefg" |         |
//   | read(0, 1)       | "adefg" | "a"     |
//   | read(0, 2)       | "adefg" | "ad"    |
//   | read(1, 4)       | "adefg" | "def"   |
//   | read(-1, 100)    | "adefg" | "adefg" |

void worker(zmq::context_t* context) {
  zmq::socket_t socket(*context, ZMQ_REP);
  const std::string address = "inproc://worker";
  socket.connect(address);

  std::string file;

  while (true) {
    const std::string msg = zmq_util::recv_string(&socket);
    std::vector<std::string> parts = fluent::Split(msg);

    if (parts.size() == 3 && parts[0] == "write") {
      const int start = std::stoi(parts[1]);
      const std::string& s = parts[2];
      file.resize(std::max(file.size(), start + s.size()), ' ');
      file.insert(start, s);
      zmq_util::send_string("OK", &socket);
    } else if (parts.size() == 3 && parts[0] == "read") {
      const int start = std::max(std::stoi(parts[1]), 0);
      const int stop =
          std::min(static_cast<std::size_t>(std::stoi(parts[2])), file.size());
      zmq_util::send_string(file.substr(start, stop - start), &socket);
    } else {
      std::string err = "ERROR: invalid request '" + msg + "'.";
      zmq_util::send_string(std::move(err), &socket);
    }
  }
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <address>" << std::endl;
    return 1;
  }

  zmq::context_t context(1);

  zmq::socket_t clients_socket(context, ZMQ_ROUTER);
  const std::string clients_address = argv[1];
  clients_socket.bind(clients_address);
  std::cout << "File system listening on " << clients_address << std::endl;

  zmq::socket_t worker_socket(context, ZMQ_DEALER);
  const std::string worker_address = "inproc://worker";
  worker_socket.bind(worker_address);
  std::thread thread(worker, &context);

  zmq::proxy(clients_socket, worker_socket, nullptr /* capture */);
}
