#ifndef FLUENT_NETWORK_STATE_H_
#define FLUENT_NETWORK_STATE_H_

namespace fluent {

// NetworkState is a simple struct holding all of the networking junk a
// FluentExecutor needs. Specifically, there are three things:
//
// 1. A zmq::context_t because all zmq networking requires a context.
// 2. A zmq:socket_t on which a FluentExecutor receives tuples from other nodes.
// 3. A SocketCache which channels use to figure out where to send messages.
//
// TODO(mwhittaker): Right now ZMQ_REP is a temporary solution. We have to
// choose the correct socket type.
struct NetworkState {
  explicit NetworkState(const std::string& address)
      : context(1), socket(context, ZMQ_REP), socket_cache(&context) {
    socket.bind(address);
    LOG(INFO) << "Fluent executor listening on '" << address << "'.";
  }

  zmq::context_t context;
  zmq::socket_t socket;
  SocketCache socket_cache;
};

}  // namespace fluent

#endif  // FLUENT_NETWORK_STATE_H_
