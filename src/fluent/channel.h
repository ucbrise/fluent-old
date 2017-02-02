#ifndef FLUENT_CHANNEL_H_
#define FLUENT_CHANNEL_H_

#include <cstddef>
#include <iostream>

#include <type_traits>
#include <utility>

#include "fluent/collection.h"
#include "fluent/serialization.h"
#include "fluent/socket_cache.h"
#include "zmq_util/zmq_util.h"

namespace fluent {

// A channel is a pseudo-relation. The first column of the channel is a string
// specifying the ZeroMQ to which the tuple should be sent. For example, if
// adding the tuple ("inproc://a", 1, 2, 3) will send the tuple ("inproc://a",
// 1, 2, 3) to the node at address ("inproc//a", 1, 2, 3).
template <typename T, typename... Ts>
class Channel : public Collection<T, Ts...> {
  static_assert(std::is_same<std::string, T>::value,
                "The first column of a channel must be a string specifying a "
                "ZeroMQ address (e.g. tcp://localhost:9999).");

 public:
  Channel(const std::string& name, SocketCache* socket_cache)
      : Collection<T, Ts...>(name), socket_cache_(socket_cache) {}

  // This method assumes a `std::string ToString(const U&)` function exists for
  // `U` in `T, Ts...`.
  void Add(const std::tuple<T, Ts...>& t) override {
    AddImpl(t, std::make_index_sequence<sizeof...(Ts) + 1>());
  }

  void Tick() override { this->MutableGet().clear(); }

 private:
  // Each tuple is sent as a multi-part ZeroMQ message. The first part is the
  // name of the Channel. Then, we sent one part for each column.
  template <std::size_t... Is>
  void AddImpl(const std::tuple<T, Ts...>& t, std::index_sequence<Is...>) {
    zmq::socket_t& socket = socket_cache_->At(std::get<0>(t));
    std::vector<std::string> strings = {ToString(std::get<Is>(t))...};
    std::vector<zmq::message_t> msgs;
    msgs.push_back(string_to_message(this->Name()));
    for (const std::string& s : strings) {
      msgs.push_back(string_to_message(s));
    }
    send_msgs(std::move(msgs), &socket);
  }

  // Whenever a tuple with address `a` is added to a Channel, the socket
  // associated with `a` in `socket_cache_` is used to send the tuple.
  SocketCache* socket_cache_;
};

}  // namespace fluent

#endif  // FLUENT_CHANNEL_H_
