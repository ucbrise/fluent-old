#ifndef EXAMPLES_CHAT_SERVER_H_
#define EXAMPLES_CHAT_SERVER_H_

#include "postgres/connection_config.h"

#include "zmq.hpp"

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

struct ServerArgs {
  std::string server_address;
};

template <template <template <typename> class, template <typename> class>
          class PostgresClient>
int ServerMain(const ServerArgs& args,
               const fluent::postgres::ConnectionConfig& connection_config) {
  zmq::context_t context(1);
  auto f =
      fluent::fluent<PostgresClient>("chat_server", args.server_address,
                                     &context, connection_config)
          .template channel<server_address_t, client_address_t, nickname_t>(
              "connect")
          .template channel<address_t, message_t>("mcast")
          .template table<client_address_t, nickname_t>("nodelist")
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

#endif  // EXAMPLES_CHAT_SERVER_H_
