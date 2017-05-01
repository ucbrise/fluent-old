#ifndef FLUENT_CHANNEL_H_
#define FLUENT_CHANNEL_H_

#include <cstddef>

#include <algorithm>
#include <array>
#include <set>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/macros.h"
#include "common/status.h"
#include "common/type_traits.h"
#include "fluent/mock_pickler.h"
#include "fluent/rule_tags.h"
#include "fluent/socket_cache.h"
#include "ra/iterable.h"
#include "zmq_util/zmq_util.h"

namespace fluent {
namespace detail {

// See `GetParser`.
template <template <typename> class Pickler, typename... Ts, std::size_t... Is>
std::tuple<Ts...> parse_tuple_impl(const std::vector<std::string>& columns,
                                   std::index_sequence<Is...>) {
  return {Pickler<Ts>().Load(columns[Is])...};
}

// See `GetParser`.
template <template <typename> class Pickler, typename... Ts>
std::tuple<Ts...> parse_tuple(const std::vector<std::string>& columns) {
  using Indices = std::make_index_sequence<sizeof...(Ts)>;
  return parse_tuple_impl<Pickler, Ts...>(columns, Indices());
}

}  // namespace detail

// Fluent nodes send tuples to one another. A parser is responsible for parsing
// serialized tuples and processing them. More specifically, a parser is given
// the node id of the sending node, the time at which the tuple was sent, the
// channel to which the tuple was sent, the stringified columns of the tuple,
// and the local time.
using Parser = std::function<Status(
    std::size_t dep_node_id, int dep_time, const std::string& channel_name,
    const std::vector<std::string>& columns, int time)>;

// A channel is a pseudo-relation. The first column of the channel is a string
// specifying the ZeroMQ to which the tuple should be sent. For example, if
// adding the tuple ("inproc://a", 1, 2, 3) will send the tuple ("inproc://a",
// 1, 2, 3) to the node at address ("inproc//a", 1, 2, 3).
template <template <typename> class Pickler, typename T, typename... Ts>
class Channel {
  static_assert(std::is_same<std::string, T>::value,
                "The first column of a channel must be a string specifying a "
                "ZeroMQ address (e.g. tcp://localhost:9999).");

 public:
  Channel(std::size_t id, std::string name,
          std::array<std::string, 1 + sizeof...(Ts)> column_names,
          SocketCache* socket_cache)
      : id_(id),
        name_(std::move(name)),
        column_names_(std::move(column_names)),
        socket_cache_(socket_cache) {}
  Channel(Channel&&) = default;
  Channel& operator=(Channel&&) = default;
  DISALLOW_COPY_AND_ASSIGN(Channel);

  const std::string& Name() const { return name_; }

  const std::array<std::string, 1 + sizeof...(Ts)>& ColumnNames() const {
    return column_names_;
  }

  const std::set<std::tuple<T, Ts...>>& Get() const { return ts_; }

  ra::Iterable<std::set<std::tuple<T, Ts...>>> Iterable() const {
    return ra::make_iterable(name_, &ts_);
  }

  void Merge(const std::set<std::tuple<T, Ts...>>& ts, int time) {
    for (const std::tuple<T, Ts...>& t : ts) {
      VLOG(1) << "Channel " << this->Name() << " sending tuple to "
              << std::get<0>(t) << ".";

      std::vector<zmq::message_t> msgs;
      msgs.push_back(zmq_util::string_to_message(ToString(id_)));
      msgs.push_back(zmq_util::string_to_message(this->Name()));
      msgs.push_back(zmq_util::string_to_message(ToString(time)));
      auto strings = TupleMap(t, [this](const auto& x) { return ToString(x); });
      TupleIter(strings, [&msgs](const std::string& s) {
        msgs.push_back(zmq_util::string_to_message(s));
      });

      zmq::socket_t& socket = socket_cache_->At(std::get<0>(t));
      zmq_util::send_msgs(std::move(msgs), &socket);
    }
  }

  std::set<std::tuple<T, Ts...>> Tick() {
    std::set<std::tuple<T, Ts...>> ts;
    std::swap(ts, ts_);
    return ts;
  }

  // `GetParser(f)` returns a parser (see Parser above) which parses a tuple
  // destined for this channel and then invokes `f`.
  template <typename F>
  Parser GetParser(F f) {
    return [this, f](std::size_t dep_node_id, int dep_time,
                     const std::string& channel_name,
                     const std::vector<std::string>& columns, int time) {
      const auto t = detail::parse_tuple<Pickler, T, Ts...>(columns);
      RETURN_IF_ERROR(f(dep_node_id, dep_time, channel_name, t, time));
      ts_.insert(t);
      return Status::OK;
    };
  }

 private:
  template <typename U>
  std::string ToString(const U& x) {
    return Pickler<typename std::decay<U>::type>().Dump(x);
  }

  const std::size_t id_;
  const std::string name_;
  const std::array<std::string, 1 + sizeof...(Ts)> column_names_;
  std::set<std::tuple<T, Ts...>> ts_;

  // Whenever a tuple with address `a` is added to a Channel, the socket
  // associated with `a` in `socket_cache_` is used to send the tuple.
  SocketCache* socket_cache_;

  FRIEND_TEST(Channel, TickClearsChannel);
};

}  // namespace fluent

#endif  // FLUENT_CHANNEL_H_
