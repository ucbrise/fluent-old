#include "examples/chat/ping_client.h"

#include <chrono>

#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "postgres/client.h"
#include "ra/all.h"

namespace postgres = fluent::postgres;
namespace ra = fluent::ra;

using address_t = std::string;
using server_address_t = std::string;
using client_address_t = std::string;
using nickname_t = std::string;
using message_t = std::string;

int PingClientMain(const PingClientArgs& args,
                   fluent::postgres::Client* postgres_client) {
  zmq::context_t context(1);

  std::vector<std::tuple<server_address_t, client_address_t, nickname_t>>
      connect_tuple = {std::make_tuple(args.server_address, args.client_address,
                                       args.nickname)};

  auto f =
      fluent::fluent("chat_ping_client", args.client_address, &context,
                     postgres_client)
          .channel<server_address_t, client_address_t, nickname_t>("connect")
          .channel<address_t, message_t>("mcast")
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
