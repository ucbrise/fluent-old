#ifndef EXAMPLES_CHAT_CLIENT_H_
#define EXAMPLES_CHAT_CLIENT_H_

#include <vector>

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

struct ClientArgs {
  std::string server_address;
  std::string client_address;
  std::string nickname;
};

template <template <template <typename> class, template <typename> class>
          class PostgresClient>
int ClientMain(const ClientArgs& args,
               const fluent::postgres::ConnectionConfig& connection_config) {
  zmq::context_t context(1);

  std::vector<std::tuple<server_address_t, client_address_t, nickname_t>>
      connect_tuple = {std::make_tuple(args.server_address, args.client_address,
                                       args.nickname)};

  auto f =
      fluent::fluent<PostgresClient>("chat_client", args.client_address,
                                     &context, connection_config)
          .stdin()
          .stdout()
          .template channel<server_address_t, client_address_t, nickname_t>(
              "connect")
          .template channel<address_t, message_t>("mcast")
          .RegisterBootstrapRules([&](auto&, auto&, auto& connect, auto&) {
            using namespace fluent::infix;
            return std::make_tuple(
                connect <= ra::make_iterable("connect_tuple", &connect_tuple));
          })
          .RegisterRules([&](auto& in, auto& out, auto&, auto& mcast) {
            using namespace fluent::infix;
            auto from_in =
                mcast <= (in.Iterable() |
                          ra::map([&](const std::tuple<std::string>& line) {
                            return std::make_tuple(args.server_address,
                                                   std::get<0>(line));
                          }));

            auto to_out = out <= (mcast.Iterable() | ra::project<1>());

            return std::make_tuple(from_in, to_out);
          });

  f.Run();
  return 0;
}

#endif  // EXAMPLES_CHAT_CLIENT_H_
