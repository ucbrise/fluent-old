#include <chrono>
#include <iostream>

#include "glog/logging.h"

#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "ra/all.h"

namespace ra = fluent::ra;

using address_t = std::string;
using server_address_t = std::string;
using client_address_t = std::string;
using nickname_t = std::string;
using message_t = std::string;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 5) {
    std::cerr << "usage: " << argv[0]
              << " <server address> <client address> <nickname> <msg>"
              << std::endl;
    return 1;
  }

  const std::string server_address = argv[1];
  const std::string client_address = argv[2];
  const std::string nickname = argv[3];
  const std::string msg = argv[4];
  zmq::context_t context(1);

  std::vector<std::tuple<server_address_t, client_address_t, nickname_t>>
      connect_tuple = {std::make_tuple(server_address, client_address, nickname)};

  auto f =
      fluent::fluent(client_address, &context)
          .channel<server_address_t, client_address_t, nickname_t>("connect")
          .channel<address_t, message_t>("mcast")
          .periodic("p", std::chrono::milliseconds(1000))
          .RegisterBootstrapRules([&](auto& connect, auto&, auto&) {
            using namespace fluent::infix;
            return std::make_tuple(connect <=
                                   ra::make_iterable(&connect_tuple));
          })
          .RegisterRules([&](auto&, auto& mcast, auto& p) {
            using namespace fluent::infix;
            return std::make_tuple(
                mcast <= (p.Iterable() | ra::map([&](const auto&) {
                            return std::make_tuple(server_address, msg);
                          })));
          });

  f.Run();
}
