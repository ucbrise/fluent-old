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
  const int delay = 0;
  zmq::context_t context(1);

  std::vector<std::tuple<address_t>>
    self_address_tuple = {std::make_tuple(self_address)};

  std::vector<std::tuple<address_t>>
    seed_address_tuple = {std::make_tuple(seed_node_address)};

  std::vector<std::tuple<int, int, int, int, bool>>
    training_data = {std::make_tuple(0, 1, 0, 3, true),
                     std::make_tuple(0, 0, 5, 6, false),
                     std::make_tuple(0, 7, 8, 0, true)};


  auto f=
    fluent::fluent(self_address, &context)
    .table<address_t>("servers")
    .channel<address_t, node_type_t, address_t>("join_request")
    .channel<address_t, std::set<std::tuple<node_type_t, address_t>>>("membership_channel")
    .channel<address_t, int, std::set<std::tuple<int, int>>>("gradient_channel")
    .lattice<MaxLattice<int>>("my_iter_num")
    .lattice<MaxLattice<int>>("server_iter_num")
    .periodic("push_trigger", std::chrono::milliseconds(1))
    .RegisterBootstrapRules([&](auto&, auto& join_request, auto&, auto&, auto&, auto&, auto&) {
            using namespace fluent::infix;

            auto send_join_request = 
                join_request <= (ra::make_iterable(&seed_address_tuple) | ra::map([&](const& t) {
                    return std::make_tuple(std::get<0>(t), 1, self_address);
                }));

            return std::make_tuple(send_join_request);
    })
    .RegisterRules([&](auto& servers, auto& join_request, auto& membership_channel, auto& gradient_channel, auto& my_iter_num, auto& server_iter_num, auto& push_trigger) {
            using namespace fluent::infix;

            auto receive_servers =
                servers <= (membership_channel.Iterable() | ra::project<1>() | ra::unbatch()
                                | ra::filter([](const auto& t) {
                                    return std::get<0>(t) == 0;
                                  })
                                | ra::project<1>());

            auto push =
                gradient_channel <= (ra::make_cross(servers.Iterable(),
                        (ra::make_cross(push_trigger.Iterable(), ra::make_iterable(training_data)) | ra::group_by<ra::Keys<2>, ra::agg::Sum<3>, ra::agg::Sum<4>,
                              ra::agg::Sum<5>>()
                        | ra::project<1, 2, 3>()
                        | ra::map([](const auto& t) {
                            std::set<std::tuple<int, int>> s;
                            TupleIteri(t, [&s](std::size_t i, auto g) {
                                            s.insert(std::make_tuple(i, g));
                                           });
                            return std::make_tuple(s);
                        })
                        | ra::map([](const auto& t) {
                            return std::make_tuple(my_iter_num.Reveal(), std::get<0>(t));
                          }))))

            return std::make_tuple(add_membership, broadcast_membership);
    });

  f.Run();
}