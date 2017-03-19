#include "examples/chat/client.h"

#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "postgres/client.h"
#include "ra/all.h"

namespace ra = fluent::ra;

using address_t = std::string;
using server_address_t = std::string;
using client_address_t = std::string;
using nickname_t = std::string;
using message_t = std::string;

int ClientMain(const ClientArgs& args,
               fluent::postgres::Client* postgres_client) {
  zmq::context_t context(1);

  std::vector<std::tuple<server_address_t, client_address_t, nickname_t>>
      connect_tuple = {std::make_tuple(args.server_address, args.client_address,
                                       args.nickname)};

  auto f =
      fluent::fluent(args.client_address, &context, postgres_client)
          .stdin()
          .stdout()
          .channel<server_address_t, client_address_t, nickname_t>("connect")
          .channel<address_t, message_t>("mcast")
          .RegisterBootstrapRules([&](auto&, auto&, auto& connect, auto&) {
            using namespace fluent::infix;
            return std::make_tuple(connect <=
                                   ra::make_iterable(&connect_tuple));
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
