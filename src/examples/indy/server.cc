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
using k_t = std::string;
using v_t = int;
using succeed_t = std::string;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 3) {
    std::cerr << "usage: " << argv[0] << " <server address> <seed address>" << std::endl;
    return 1;
  }

  const std::string server_address = argv[1];
  const std::string seed_address = argv[2];
  zmq::context_t context(1);

  std::vector<std::tuple<server_address_t, server_address_t>>
    connect_tuple = {std::make_tuple(seed_address, server_address)};

  std::vector<std::tuple<std::string, int>>
    test_tuple = {std::make_tuple("A", 10)};

  auto f =
      fluent::fluent(server_address, &context)
          // Keeps track of a list of server address for gossiping
          .table<server_address_t>("serverlist")
          // Keeps track of a set of keys that get changed within a gossip period
          .table<k_t>("changeset")
          // Communication from a new worker node to the seed node
          .channel<server_address_t, server_address_t>("addrgossipseed")
          // Communication from the seed node to a new worker node
          .channel<server_address_t, server_address_t>("addrgossip")
          // New worker node sync up its KVS with the seed node
          .channel<server_address_t, k_t, v_t>("sync")
          // Receive GET request
          .channel<server_address_t, client_address_t, k_t>("getrequest")
          // Respond to GET request
          .channel<client_address_t, v_t>("getresponse")
          // Receive PUT request
          .channel<server_address_t, client_address_t, k_t, v_t>("putrequest")
          // Respond to PUT request
          .channel<client_address_t, succeed_t>("putresponse")
          // Gossip updates to other worker nodes
          .channel<server_address_t, k_t, v_t>("gossip")
          // KVS using MaxLattice for conflict resolution
          .lattice<fluent::MapLattice<std::string, fluent::MaxLattice<int>>>("mapl")
          // Trigger gossip every 100 milliseconds
          .periodic("p", std::chrono::milliseconds(100))
          .RegisterBootstrapRules([&](auto& serverlist, auto&, auto& addrgossipseed, auto&, auto&, auto&, auto&, auto&, auto&, auto&, auto&, auto&) {
            using namespace fluent::infix;

            auto add_self = 
                serverlist <= (ra::make_iterable(&connect_tuple) | ra::project<1>());

            auto to_seed = 
                addrgossipseed <=
                (ra::make_iterable(&connect_tuple) | ra::filter([](const std::tuple<server_address_t, server_address_t>& t) {
                                                       return std::get<0>(t) != std::get<1>(t);
                                                     }));
            return std::make_tuple(add_self, to_seed);
          })
          .RegisterRules([&](auto& serverlist, auto& changeset, auto& addrgossipseed, auto& addrgossip, auto& sync, auto& getrequest, auto& getresponse, auto& putrequest, auto& putresponse, auto& gossip, auto& mapl, auto& p) {
            using namespace fluent::infix;

            auto receive_addr_seed = 
                serverlist <= (addrgossipseed.Iterable() | ra::project<1>());

            // can potentially insert duplicate entries, which is inefficient
            auto send_addr_seed =
                addrgossip <= (ra::make_cross(ra::make_cross(addrgossipseed.Iterable(), (serverlist.Iterable() | ra::filter([&](const std::tuple<server_address_t>& t) {
                                                                                                                    return std::get<0>(t) != server_address;
                                                                                                                  })))
                                             , serverlist.Iterable())
                              | ra::map([](const std::tuple<server_address_t, server_address_t, server_address_t, server_address_t>& t) {
                                  return std::make_tuple(std::get<2>(t), std::get<3>(t));
                                }));

            auto send_sync =
                sync <= 
                (ra::make_cross((addrgossipseed.Iterable() | ra::project<1>()), mapl.Iterable()));

            auto receive_sync =
                mapl <=
                (sync.Iterable() | ra::project<1,2>());

            auto receive_addr = 
                serverlist <= (addrgossip.Iterable() | ra::project<1>());

            auto processget =
                getresponse <=
                (getrequest.Iterable() | ra::project<1,2>() |
                 ra::map([&mapl](const std::tuple<client_address_t, k_t>& t) {
                   return std::make_tuple(std::get<0>(t), mapl.at(std::get<1>(t)).Reveal());
                 }));

            auto processput =
                putresponse <=
                (putrequest.Iterable() | ra::project<1,2,3>() |
                 ra::map([&mapl](const std::tuple<client_address_t, k_t, v_t>& t) {
                   mapl.merge(std::get<1>(t), std::get<2>(t));
                   std::string result ("PUT Succeeded");
                   return std::make_tuple(std::get<0>(t), result);
                 }));

            auto updatechangeset =
                changeset <= 
                (putrequest.Iterable() | ra::project<2>());

            auto sendgossip =
                gossip <=
                (ra::make_cross(p.Iterable(), 
                                ra::make_cross((serverlist.Iterable() | ra::filter([&](const std::tuple<server_address_t>& t) {
                                                           return std::get<0>(t) != server_address;
                                                         }))
                                              , changeset.Iterable()))
                 | ra::map([&mapl](const auto& t) {
                     return std::make_tuple(std::get<2>(t), std::get<3>(t), mapl.at(std::get<3>(t)).Reveal());
                   }));

            auto clearchangeset =
                changeset -=
                (ra::make_cross(p.Iterable(), changeset.Iterable())
                 | ra::map([](const auto& t) {
                     return std::make_tuple(std::get<2>(t));
                   }));

            auto receivegossip =
                mapl <=
                (gossip.Iterable() | ra::project<1,2>());


            return std::make_tuple(receive_addr_seed, send_addr_seed, send_sync, receive_sync, receive_addr, processget, processput, updatechangeset, sendgossip, clearchangeset, receivegossip);
          });

  f.Run();
}