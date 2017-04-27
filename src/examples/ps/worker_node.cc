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
  const int delay = 1;
  zmq::context_t context(1);

  std::vector<std::tuple<address_t>>
    self_address_tuple = {std::make_tuple(self_address)};

  std::vector<std::tuple<address_t>>
    seed_address_tuple = {std::make_tuple(seed_node_address)};

  std::vector<std::tuple<int, double, double, double, int>>
    training_data = {std::make_tuple(0, 1.0, 0.0, 3.0, 1),
                     std::make_tuple(0, 0.0, 5.0, 6.0, 0),
                     std::make_tuple(0, 7.0, 8.0, 0.0, 1)};


  auto f=
    fluent::fluent(self_address, &context)
    .table<address_t>("servers")
    .table<int, address_t>("server_iter_count")
    .scratch<int>("push_condition")
    .channel<address_t, node_type_t, address_t>("join_request")
    .channel<address_t, std::set<std::tuple<node_type_t, address_t>>>("membership_channel")
    .channel<address_t, address_t, int, std::set<std::tuple<int, double>>>("gradient_channel")
    .channel<address_t, address_t, int, std::set<std::tuple<int, double>>>("weight_channel")
    .lattice<fluent::MaxLattice<int>>("my_iter_num")
    .lattice<fluent::MaxLattice<int>>("server_iter_num")
    .lattice<fluent::MapLattice<int, fluent::LwwLattice<int, double>>>("weight")
    .periodic("push_trigger", std::chrono::milliseconds(1))
    .RegisterBootstrapRules([&](auto&, auto&, auto&, auto& join_request, auto&, auto&, auto&, auto& my_iter_num, auto&, auto&, auto&) {
            using namespace fluent::infix;

            auto send_join_request = 
                join_request <= (ra::make_iterable(&seed_address_tuple) | ra::map([&](const auto& t) {
                    return std::make_tuple(std::get<0>(t), 1, self_address);
                }));

            auto initialize_my_iter_num =
                my_iter_num <= fluent::MaxLattice<int>(1);

            return std::make_tuple(send_join_request, initialize_my_iter_num);
    })
    .RegisterRules([&](auto& servers, auto& server_iter_count, auto& push_condition, auto&, auto& membership_channel, auto& gradient_channel, auto& weight_channel, auto& my_iter_num, auto& server_iter_num, auto& weight, auto& push_trigger) {
            using namespace fluent::infix;

            auto receive_servers =
                servers <= (membership_channel.Iterable() | ra::project<1>() | ra::unbatch()
                                | ra::filter([](const auto& t) {
                                    return std::get<0>(t) == 0;
                                  })
                                | ra::project<1>());

            auto update_push_condition =
                push_condition <= (push_trigger.Iterable() | ra::filter([&my_iter_num, &server_iter_num, &delay](const auto&) {return my_iter_num.Reveal() - server_iter_num.Reveal() <= delay;})
                                                           | ra::map([](const auto&) {
                                                               // dummy tuple
                                                               return std::make_tuple(1);
                                                             }));

            // auto update_temp_weight =
            //     temp_weight <= (ra::make_cross(push_condition.Iterable(), weight.Iterable().sort()) | ra::project<3>());

            // auto push =
            //     gradient_channel <= (ra::make_cross(push_condition.Iterable(), 
            //                                         ra::make_iterable(training_data))
            //                         | ra::map([&weight](const auto& t) {

            //                             vec x(3);
            //                             TupleIteri(t, [&x](std::size_t i, auto g) {
            //                                             x(i) = g;
            //                                           });
            //                             x * x.t * w
            //                           })


            auto push =
                gradient_channel <= (ra::make_cross(servers.Iterable(),
                        (ra::make_cross(push_condition.Iterable(),
                                       ra::make_iterable(&training_data))
                        | ra::group_by<ra::Keys<1>, ra::agg::Sum<2>, ra::agg::Sum<3>, ra::agg::Sum<4>>()
                        | ra::project<1, 2, 3>()
                        | ra::map([](const auto& t) {
                            // Transpose?? featureid not always aligned with index
                            std::set<std::tuple<int, int>> s;
                            fluent::TupleIteri(t, [&s](std::size_t i, auto g) {
                                            s.insert(std::make_tuple(i, g));
                                           });
                            return std::make_tuple(s);
                        })
                        | ra::map([&self_address, &my_iter_num](const auto& t) {
                            return std::make_tuple(self_address, my_iter_num.Reveal(), std::get<0>(t));
                          }))));

            auto advance_iter_num =
                my_iter_num <= (push_condition.Iterable() | ra::map([&my_iter_num](const auto&) {
                                                            return std::make_tuple(fluent::MaxLattice<int>(my_iter_num.Reveal() + 1));
                                                          }));

            auto update_weight =
                weight <= (weight_channel.Iterable() | ra::map([](const auto& t) {
                                                        auto iter_num = std::get<2>(t);
                                                        std::set<std::tuple<int, int, double>> batch;
                                                        for (auto it = std::get<3>(t).begin(); it != std::get<3>(t).end(); it++) {
                                                            batch.insert(std::make_tuple(iter_num, std::get<0>(*it), std::get<1>(*it)));
                                                        }
                                                        return std::make_tuple(batch);
                                                      })
                                                     | ra::unbatch()
                                                     | ra::map([](const auto& t) {
                                                         fluent::LwwLattice<int, double> l(std::pair<int, double>(std::get<0>(t), std::get<2>(t)));
                                                         fluent::MapLattice<int, fluent::LwwLattice<int, double>> mapl;
                                                         mapl.insert_pair(std::get<1>(t), l);
                                                         return std::make_tuple(mapl);
                                                       }));

            auto update_server_iter_count =
                server_iter_count <= (weight_channel.Iterable() | ra::project<2, 1>());

            auto update_server_iter_num =
                server_iter_num <= (server_iter_count.Iterable() | ra::group_by<ra::Keys<0>, ra::agg::Count<1>>()
                                                                 | ra::filter([&servers](const auto& t) {
                                                                    // Should users be able to do things like that?
                                                                     return std::get<1>(t) == servers.Get().size();
                                                                   })
                                                                 | ra::map([](const auto& t) {
                                                                     return std::make_tuple(fluent::MaxLattice<int>(std::get<0>(t)));   
                                                                   }));

            auto remove_server_iter =
                server_iter_count -= (server_iter_count.Iterable() | ra::filter([&server_iter_num](const auto& t) {
                                                                      return std::get<0>(t) <= server_iter_num.Reveal();
                                                                    }));
            

            return std::make_tuple(receive_servers, update_push_condition, push, advance_iter_num, update_weight, update_server_iter_count, update_server_iter_num, remove_server_iter);
    });

  f.Run();
}