#include "examples/chat/server.h"

#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "postgres/connection_config.h"
#include "ra/all.h"

namespace ra = fluent::ra;

using address_t = std::string;
using server_address_t = std::string;
using client_address_t = std::string;
using nickname_t = std::string;
using message_t = std::string;

int ServerMain(const ServerArgs& args,
               fluent::postgres::Client* postgres_client) {
  zmq::context_t context(1);
  auto f =
      fluent::fluent(args.server_address, &context, postgres_client)
          .channel<server_address_t, client_address_t, nickname_t>("connect")
          .channel<address_t, message_t>("mcast")
          .table<client_address_t, nickname_t>("nodelist")
          .RegisterRules([&](auto& connect, auto& mcast, auto& nodelist) {
            using namespace fluent::infix;

            auto subscribe =
                nodelist <= (connect.Iterable() | ra::project<1, 2>());

            // TODO(mwhittaker): Currently, nicknames are not being used. We
            // should prepend the nickaname of the sender to the message before
            // broadcasting it.
            auto multicast = mcast <= (ra::make_cross(mcast.Iterable(),
                                                      nodelist.Iterable()) |
                                       ra::project<2, 1>());

            return std::make_tuple(subscribe, multicast);
          });

  f.Run();
  return 0;
}
