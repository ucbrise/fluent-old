#ifndef EXAMPLES_CHAT_PING_CLIENT_H_
#define EXAMPLES_CHAT_PING_CLIENT_H_

#include <chrono>
#include <vector>

#include "zmq.hpp"

#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "postgres/connection_config.h"
#include "ra/all.h"

namespace postgres = fluent::postgres;
namespace ra = fluent::ra;

using address_t = std::string;
using server_address_t = std::string;
using client_address_t = std::string;
using nickname_t = std::string;
using message_t = std::string;

struct PingClientArgs {
  std::string server_address;
  std::string client_address;
  std::string nickname;
  std::string msg;
};

template <template <template <typename> class, template <typename> class>
          class PostgresClient>
int PingClientMain(
    const PingClientArgs& args,
    const fluent::postgres::ConnectionConfig& connection_config) {
  zmq::context_t context(1);

  std::vector<std::tuple<server_address_t, client_address_t, nickname_t>>
      connect_tuple = {std::make_tuple(args.server_address, args.client_address,
                                       args.nickname)};

  auto f =
      fluent::fluent<PostgresClient>("chat_ping_client", args.client_address,
                                     &context, connection_config)
          .template channel<server_address_t, client_address_t, nickname_t>(
              "connect")
          .template channel<address_t, message_t>("mcast")
          .periodic("p", std::chrono::milliseconds(1000))
          .RegisterBootstrapRules([&](auto& connect, auto&, auto&) {
            using namespace fluent::infix;
            return std::make_tuple(connect <=
                                   ra::make_iterable(&connect_tuple));
          })
          .RegisterRules([&](auto&, auto& mcast, auto& p) {
            using namespace fluent::infix;
            return std::make_tuple(mcast <=
                                   (p.Iterable() | ra::map([&](const auto&) {
                                      return std::make_tuple(
                                          args.server_address, args.msg);
                                    })));
          });

  f.Run();
  return 0;
}

#endif  // EXAMPLES_CHAT_PING_CLIENT_H_
