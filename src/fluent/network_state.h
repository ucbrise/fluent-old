#ifndef FLUENT_NETWORK_STATE_H_
#define FLUENT_NETWORK_STATE_H_

#include <string>

#include "zmq.hpp"

#include "zmq_util/socket_cache.h"

namespace fluent {

// NetworkState is a simple struct holding all of the networking junk a
// FluentExecutor needs. Specifically, there are three things:
//
// 1. A zmq::context_t because all zmq networking requires a context.
// 2. A PULL zmq:socket_t on which a FluentExecutor receives from other nodes.
// 3. A SocketCache which channels use to figure out where to send messages.
struct NetworkState {
  explicit NetworkState(const std::string& address,
                        zmq::context_t* const context_)
      : context(context_), socket(*context, ZMQ_PULL), socket_cache(context) {
    socket.bind(address);
    LOG(INFO) << "Fluent executor listening on '" << address << "'.";
  }

  zmq::context_t* const context;
  zmq::socket_t socket;
  zmq_util::SocketCache socket_cache;
};

}  // namespace fluent

#endif  // FLUENT_NETWORK_STATE_H_
