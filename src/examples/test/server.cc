#include <iostream>

#include "glog/logging.h"

#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "ra/all.h"

namespace ra = fluent::ra;

using address_t = std::string;
using node_type_t = int;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 3) {
    std::cerr << "usage: " << argv[0] << " <server address> <client address>" << std::endl;
    return 1;
  }

  const address_t server_address = argv[1];
  const address_t client_address = argv[2];
  std::set<std::tuple<node_type_t, address_t>> xs = {{1, "A"}, {2, "B"}, {3, "C"}};
  std::set<std::tuple<address_t>> client_addr_tuple = {std::make_tuple(client_address)};
  zmq::context_t context(1);

  auto f =
      fluent::fluent(server_address, &context)
          .table<node_type_t, address_t>("membership")
          .channel<address_t, std::set<std::tuple<node_type_t, address_t>>>("connect")
          .RegisterBootstrapRules([&](auto& membership, auto& connect) {
            using namespace fluent::infix;

            auto populate = 
                membership <= (ra::make_iterable(&xs));

            auto send = 
                connect <= (ra::make_cross(ra::make_iterable(&client_addr_tuple),
                                          (membership.Iterable() | ra::batch())));

            return std::make_tuple(populate, send);
          })
          .RegisterRules([&](auto&, auto&) {
            return std::make_tuple();
          });

  f.Run();
}
