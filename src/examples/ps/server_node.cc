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
  const double step_size = 0.1;
  zmq::context_t context(1);

  std::vector<std::tuple<address_t>>
    self_address_tuple = {std::make_tuple(self_address)};

  std::vector<std::tuple<address_t>>
    seed_address_tuple = {std::make_tuple(seed_node_address)};


  auto f=
    fluent::fluent(self_address, &context)
    .table<address_t>("workers")
    .scratch<size_t>("weight_condition")
    .channel<address_t, node_type_t, address_t>("join_request")
    .channel<address_t, std::set<std::tuple<node_type_t, address_t>>>("membership_channel")
    .channel<address_t, address_t, int, std::set<std::tuple<int, double>>>("gradient_channel")
    .channel<address_t, address_t, int, std::set<std::tuple<int, double>>>("weight_channel")
    .lattice<fluent::MaxLattice<int>>("server_iter_num")
    .lattice<fluent::MapLattice<int, fluent::SetLattice<address_t, int, double>>>("gradient")
    .lattice<fluent::MapLattice<int, fluent::LwwLattice<int, double>>>("weight")
    .RegisterBootstrapRules([&](auto&, auto&, auto& join_request, auto&, auto&, auto&, auto& server_iter_num, auto&, auto&) {
            using namespace fluent::infix;

            auto send_join_request = 
                join_request <= (ra::make_iterable(&seed_address_tuple) | ra::map([&](const auto& t) {
                    return std::make_tuple(std::get<0>(t), 0, self_address);
                }));

            auto initialize_server_iter_num =
                server_iter_num <= fluent::MaxLattice<int>(1);

            return std::make_tuple(send_join_request, initialize_server_iter_num);
    })
    .RegisterRules([&](auto& workers, auto& weight_condition, auto&, auto& membership_channel, auto& gradient_channel, auto& weight_channel, auto& server_iter_num, auto& gradient, auto& weight) {
            using namespace fluent::infix;

            auto receive_workers =
                workers <= (membership_channel.Iterable() | ra::project<1>() | ra::unbatch()
                                | ra::filter([](const auto& t) {
                                    return std::get<0>(t) == 1;
                                  })
                                | ra::project<1>());

            auto receive_gradient =
                gradient <= (gradient_channel.Iterable() | ra::map([](const auto& t) {
                                                            auto worker_addr = std::get<1>(t);
                                                            auto iter_num = std::get<2>(t);
                                                            std::set<std::tuple<address_t, int, int, double>> batch;
                                                            for (auto it = std::get<3>(t).begin(); it != std::get<3>(t).end(); it++) {
                                                                batch.insert(std::make_tuple(worker_addr, iter_num, std::get<0>(*it), std::get<1>(*it)));
                                                            }
                                                            return std::make_tuple(batch);
                                                          })
                                                        | ra::unbatch()
                                                        | ra::map([](const auto& t) {
                                                            fluent::SetLattice<address_t, int, double> s;
                                                            s.merge(std::make_tuple(std::get<0>(t), std::get<2>(t), std::get<3>(t)));
                                                            fluent::MapLattice<int, fluent::SetLattice<address_t, int, double>> mapl;
                                                            mapl.insert_pair(std::get<1>(t), s);
                                                            return std::make_tuple(mapl);
                                                          }));

            auto update_weight_condition =
                weight_condition <= (gradient.at(server_iter_num.Reveal()).Iterable() | ra::group_by<ra::Keys<0>, ra::agg::Count<1>>() | ra::count() 
                                                                                      | ra::filter([&workers](const auto& t) {
                                                                                          return std::get<0>(t) == workers.Get().size();
                                                                                        }));

            auto update_weight = 
                weight <= (ra::make_cross(weight_condition.Iterable(), 
                                         (gradient.at(server_iter_num.Reveal()).Iterable() | ra::group_by<ra::Keys<1>, ra::agg::Sum<2>>()))
                              | ra::project<1, 2>()
                              | ra::map([&step_size, &weight, &server_iter_num](const auto& t) {
                                  double new_weight = weight.at(std::get<0>(t)).Reveal().second + step_size * std::get<1>(t);
                                  fluent::LwwLattice<int, double> l(std::pair<int, double>(server_iter_num.Reveal(), new_weight));
                                  fluent::MapLattice<int, fluent::LwwLattice<int, double>> mapl;
                                  mapl.insert_pair(std::get<0>(t), l);
                                      return std::make_tuple(mapl);
                                }));

            auto send_weight =
                weight_channel <= (ra::make_cross(workers.Iterable(),
                                                (ra::make_cross(weight_condition.Iterable(),
                                                                weight.Iterable())
                                                 | ra::map([](const auto& t) {
                                                     return std::make_tuple(std::get<1>(t), std::get<2>(t).Reveal().second);
                                                   })
                                                 | ra::batch()))
                                  | ra::map([&self_address, &server_iter_num](const auto& t) {
                                      return std::make_tuple(std::get<0>(t), self_address, server_iter_num.Reveal(), std::get<1>(t));
                                    }));
              
            auto update_server_iter_num =
                server_iter_num <= (weight_condition.Iterable() | ra::map([&server_iter_num](const auto&) {
                                                                   return std::make_tuple(fluent::MaxLattice<int>(server_iter_num.Reveal() + 1));
                                                                 }));

            return std::make_tuple(receive_workers, receive_gradient, update_weight_condition, update_weight, send_weight, update_server_iter_num);
    });

  f.Run();
}