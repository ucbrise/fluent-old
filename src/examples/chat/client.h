#ifndef EXAMPLES_CHAT_CLIENT_H_
#define EXAMPLES_CHAT_CLIENT_H_

#include <vector>

#include "zmq.hpp"

#include "common/status.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "ra/all.h"

namespace ra = fluent::ra;

struct ClientArgs {
  std::string server_address;
  std::string client_address;
  std::string nickname;
};

template <template <template <typename> class, template <typename> class>
          class LineageDbClient>
int ClientMain(const ClientArgs& args,
               const fluent::lineagedb::ConnectionConfig& connection_config) {
  using connect_tuple_t = std::tuple<std::string, std::string, std::string>;
  std::vector<connect_tuple_t> connect_tuple = {
      std::make_tuple(args.server_address, args.client_address, args.nickname)};

  zmq::context_t context(1);
  fluent::Status status =
      fluent::fluent<LineageDbClient>("chat_client_" + args.nickname,
                                      args.client_address, &context,
                                      connection_config)
          .ConsumeValueOrDie()
          .stdin()
          .stdout()
          .template channel<std::string, std::string, std::string>(
              "connect", {{"server_addr", "client_addr", "nickname"}})
          .template channel<std::string, std::string>("mcast",
                                                      {{"addr", "msg"}})
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
          })
          .ConsumeValueOrDie()
          .Run();
  CHECK_EQ(fluent::Status::OK, status);

  return 0;
}

#endif  // EXAMPLES_CHAT_CLIENT_H_
