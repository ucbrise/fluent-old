#include <iostream>
#include <sstream>

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
    std::cerr << "usage: " << argv[0] << " <address> <seed_address>" << std::endl;
    return 1;
  }

  const address_t self_address = argv[1];
  const address_t seed_node_address = argv[2];
  zmq::context_t context(1);

  std::vector<std::tuple<address_t>>
    self_address_tuple = {std::make_tuple(self_address)};

  std::vector<std::tuple<int>>
    num_nodes_tuple = {std::make_tuple(num_nodes)};

  auto f=
    fluent::fluent(self_address, &context)
    .table<node_type_t, address_t>("membership")
    .channel<address_t, node_type_t, address_t>("join_request")
    .channel<address_t, node_type_t, address_t>("outgoing_membership")
    .RegisterBootstrapRules([&](auto&, auto&, auto&) {})
    .RegisterRules([&](auto& membership, auto& join_request, auto& outgoing_membership) {
            using namespace fluent::infix;

            auto add_membership =
                membership <= (join_request.Iterable() | ra::project<1,2>());

            auto broadcast_membership =
                outgoing_membership <=
                (ra::make_cross(membership.Iterable(), 
                    (membership.Iterable() | ra::count() | ra::filter([&](const auto& t) {
                        return std::get<0>(t) == num_nodes;
                    })))
                | ra::project<0, 1>())

            return std::make_tuple(add_membership, broadcast_membership);
    });

  f.Run();
}