#ifndef EXAMPLES_CHAT_SERVER_H_
#define EXAMPLES_CHAT_SERVER_H_

#include "lineagedb/connection_config.h"

#include "zmq.hpp"

#include "common/status.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "ra/all.h"

namespace ra = fluent::ra;

struct ServerArgs {
  std::string server_address;
};

template <template <template <typename> class, template <typename> class,
                    typename> class LineageDbClient>
int ServerMain(const ServerArgs& args,
               const fluent::lineagedb::ConnectionConfig& connection_config) {
  zmq::context_t context(1);
  fluent::Status status =
      fluent::fluent<LineageDbClient>("chat_server", args.server_address,
                                      &context, connection_config)
          .ConsumeValueOrDie()
          .template channel<std::string, std::string, std::string>(
              "connect", {{"server_addr", "client_addr", "nickname"}})
          .template channel<std::string, std::string>("mcast",
                                                      {{"addr", "msg"}})
          .template table<std::string, std::string>(
              "nodelist", {{"client_addr", "nickname"}})
          .RegisterRules([&](auto& connect, auto& mcast, auto& nodelist) {
            using namespace fluent::infix;

            auto subscribe =
                nodelist <= (connect.Iterable() | ra::project<1, 2>());

            // TODO(mwhittaker): Currently, nicknames are not being used.
            // We should prepend the nickaname of the sender to the
            // message before broadcasting it.
            auto multicast = mcast <= (ra::make_cross(mcast.Iterable(),
                                                      nodelist.Iterable()) |
                                       ra::project<2, 1>());

            return std::make_tuple(subscribe, multicast);
          })
          .ConsumeValueOrDie()
          .Run();
  CHECK_EQ(fluent::Status::OK, status);

  return 0;
}

#endif  // EXAMPLES_CHAT_SERVER_H_
