#ifndef EXAMPLES_CHAT_PING_CLIENT_H_
#define EXAMPLES_CHAT_PING_CLIENT_H_

#include <chrono>
#include <vector>

#include "zmq.hpp"

#include "common/status.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "ra/all.h"

namespace lineagedb = fluent::lineagedb;
namespace ra = fluent::ra;

struct PingClientArgs {
  std::string server_address;
  std::string client_address;
  std::string nickname;
  std::string msg;
};

template <template <template <typename> class, template <typename> class>
          class LineageDbClient>
int PingClientMain(const PingClientArgs& args,
                   const lineagedb::ConnectionConfig& connection_config) {
  using connect_tuple_t = std::tuple<std::string, std::string, std::string>;
  std::vector<connect_tuple_t> connect_tuple = {
      std::make_tuple(args.server_address, args.client_address, args.nickname)};

  zmq::context_t context(1);
  fluent::Status status =
      fluent::fluent<LineageDbClient>("chat_ping_client", args.client_address,
                                      &context, connection_config)
          .ConsumeValueOrDie()
          .template channel<std::string, std::string, std::string>(
              "connect", {{"server_addr", "client_addr", "nickname"}})
          .template channel<std::string, std::string>("mcast",
                                                      {{"addr", "msg"}})
          .periodic("p", std::chrono::milliseconds(1000))
          .RegisterBootstrapRules([&](auto& connect, auto&, auto&) {
            using namespace fluent::infix;
            return std::make_tuple(
                connect <= ra::make_iterable("connect_tuple", &connect_tuple));
          })
          .RegisterRules([&](auto&, auto& mcast, auto& p) {
            using namespace fluent::infix;
            return std::make_tuple(mcast <=
                                   (p.Iterable() | ra::map([&](const auto&) {
                                      return std::make_tuple(
                                          args.server_address, args.msg);
                                    })));
          })
          .ConsumeValueOrDie()
          .Run();
  CHECK_EQ(fluent::Status::OK, status);

  return 0;
}

#endif  // EXAMPLES_CHAT_PING_CLIENT_H_
