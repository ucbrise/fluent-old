#ifndef FLUENT_CHANNEL_H_
#define FLUENT_CHANNEL_H_

#include <cstddef>
#include <iostream>

#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "fluent/collection.h"
#include "fluent/rule_tags.h"
#include "fluent/serialization.h"
#include "fluent/socket_cache.h"
#include "zmq_util/zmq_util.h"

namespace fluent {

namespace detail {
// See `GetParser`.
template <typename... Ts, std::size_t... Is>
std::tuple<Ts...> parse_tuple_impl(const std::vector<std::string>& columns,
                                   std::index_sequence<Is...>) {
  return {FromString<Ts>(columns[Is])...};
}

// See `GetParser`.
template <typename... Ts>
std::tuple<Ts...> parse_tuple(const std::vector<std::string>& columns) {
  using Indices = std::make_index_sequence<sizeof...(Ts)>;
  return parse_tuple_impl<Ts...>(columns, Indices());
}
}  // namespace detail

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

  // Merge assumes a `std::string ToString(const U&)` function exists for `U`
  // in `T, Ts...`.
  template <typename RA>
  void Merge(const RA& ra) {
    MergeImpl(ra, std::make_index_sequence<sizeof...(Ts) + 1>());
  }

  template <typename Rhs>
  std::tuple<Channel<T, Ts...>*, MergeTag, typename std::decay<Rhs>::type>
  operator<=(Rhs&& rhs) {
    return {this, MergeTag(), std::forward<Rhs>(rhs)};
  }

  void Tick() override { this->MutableGet().clear(); }

  // `Collection<T1, ..., Tn>.GetParser()(columns)` parses a vector of `n`
  // strings into a tuple of type `std::tuple<T1, ..., Tn>` and inserts it into
  // the collection. The ith element of the tuple is parsed using a global `Ti
  // FromString<Ti>(const std::string&)` function which is assumed to exists
  // (see `serialization.h` for more information).
  std::function<void(const std::vector<std::string>& columns)> GetParser() {
    return [this](const std::vector<std::string>& columns) {
      this->MutableGet().insert(detail::parse_tuple<T, Ts...>(columns));
    };
  }

 private:
  template <typename RA, std::size_t... Is>
  void MergeImpl(const RA& ra, std::index_sequence<Is...>) {
    auto physical = ra.ToPhysical();
    auto rng = physical.ToRange();
    ranges::for_each(rng, [this](const std::tuple<T, Ts...>& t) {
      VLOG(1) << "Channel " << this->Name() << " sending tuple to "
              << std::get<0>(t) << ".";

      std::vector<std::string> strings = {ToString(std::get<Is>(t))...};
      std::vector<zmq::message_t> msgs;
      msgs.push_back(zmq_util::string_to_message(this->Name()));
      for (const std::string& s : strings) {
        msgs.push_back(zmq_util::string_to_message(s));
      }

      zmq::socket_t& socket = socket_cache_->At(std::get<0>(t));
      zmq_util::send_msgs(std::move(msgs), &socket);
    });
  }

  // Whenever a tuple with address `a` is added to a Channel, the socket
  // associated with `a` in `socket_cache_` is used to send the tuple.
  SocketCache* socket_cache_;

  FRIEND_TEST(Channel, TickClearsChannel);
};

}  // namespace fluent

#endif  // FLUENT_CHANNEL_H_
