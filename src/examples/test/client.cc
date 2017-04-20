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

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <client address>" << std::endl;
    return 1;
  }

  const address_t client_address = argv[1];
  std::set<std::tuple<address_t>> client_addr_tuple = {std::make_tuple(client_address)};
  zmq::context_t context(1);

  auto f =
      fluent::fluent(client_address, &context)
          .stdout()
          .table<node_type_t, address_t>("membership")
          .channel<address_t, std::set<std::tuple<node_type_t, address_t>>>("connect")
          .RegisterBootstrapRules([&](auto&, auto&, auto&) {
            return std::make_tuple();
          })
          .RegisterRules([&](auto& out, auto& membership, auto& connect) {
            using namespace fluent::infix;

            auto receive = 
                membership <= (connect.Iterable() | ra::project<1>() | ra::unbatch());

            auto to_out =
                out <= (membership.Iterable() | ra::project<1>());

            return std::make_tuple(receive, to_out);
          });

  f.Run();
}
