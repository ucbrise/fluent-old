#include <iostream>

#include "glog/logging.h"

#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "ra/all.h"

namespace ra = fluent::ra;

using address_t = std::string;
using server_address_t = std::string;
using client_address_t = std::string;
using nickname_t = std::string;
using message_t = std::string;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <server address>" << std::endl;
    return 1;
  }

  const std::string server_address = argv[1];
  zmq::context_t context(1);

  auto f =
      fluent::fluent(server_address, &context)
          .channel<server_address_t, client_address_t, nickname_t>("connect")
          .channel<address_t, message_t>("mcast")
          .table<client_address_t, nickname_t>("nodelist")
          .RegisterRules([&](auto& connect, auto& mcast, auto& nodelist) {
            auto subscribe =
                nodelist <= (connect.Iterable() | ra::project<1, 2>());

            // TODO(mwhittaker): Currently, nicknames are not being used. We
            // should prepend the nickaname of the sender to the message before
            // broadcasting it.
            auto multicast =
                mcast <=
                (ra::make_cross(mcast.Iterable(), nodelist.Iterable()) |
                 ra::map([](const std::tuple<server_address_t, message_t,
                                             client_address_t, nickname_t>& t) {
                   return std::make_tuple(std::get<2>(t), std::get<1>(t));
                 }));

            return std::make_tuple(subscribe, multicast);
          });

  f.Run();
}
