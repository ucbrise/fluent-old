#ifndef COLLETIONS_CHANNEL_H_
#define COLLETIONS_CHANNEL_H_

#include <cstddef>

#include <algorithm>
#include <array>
#include <set>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "collections/collection.h"
#include "collections/util.h"
#include "common/macros.h"
#include "common/static_assert.h"
#include "common/status.h"
#include "common/tuple_util.h"
#include "common/type_traits.h"
// #include "fluent/mock_pickler.h"
#include "zmq_util/socket_cache.h"
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

// A channel is a pseudo-relation. The first column of the channel is a string
// specifying the ZeroMQ to which the tuple should be sent. For example, if
// adding the tuple ("inproc://a", 1, 2, 3) will send the tuple ("inproc://a",
// 1, 2, 3) to the node at address ("inproc//a", 1, 2, 3).
template <template <typename> class Pickler, typename T, typename... Ts>
class Channel : public Collection {
  static_assert(StaticAssert<std::is_same<std::string, T>>::value,
                "The first column of a channel must be a string specifying a "
                "ZeroMQ address (e.g. tcp://localhost:9999).");

 public:
  Channel(std::size_t id, std::string name,
          std::array<std::string, 1 + sizeof...(Ts)> column_names,
          zmq_util::SocketCache* socket_cache)
      : id_(id),
        name_(std::move(name)),
        column_names_(std::move(column_names)),
        socket_cache_(socket_cache) {}
  DISALLOW_COPY_AND_ASSIGN(Channel);
  DEFAULT_MOVE_AND_ASSIGN(Channel);

  const std::string& Name() const { return name_; }

  const std::array<std::string, 1 + sizeof...(Ts)>& ColumnNames() const {
    return column_names_;
  }

  const std::map<std::tuple<T, Ts...>, CollectionTupleIds>& Get() const {
    return ts_;
  }

  void Merge(const std::tuple<T, Ts...>& t, std::size_t hash,
             int logical_time_inserted) {
    UNUSED(hash);

    using zmq_util::string_to_message;
    std::vector<zmq::message_t> msgs;
    msgs.push_back(string_to_message(ToString(id_)));
    msgs.push_back(string_to_message(ToString(name_)));
    msgs.push_back(string_to_message(ToString(logical_time_inserted)));

    auto to_string = [this](const auto& x) { return this->ToString(x); };
    const auto strings = TupleMap(t, to_string);
    TupleIter(strings, [&msgs](const std::string& s) {
      msgs.push_back(zmq_util::string_to_message(s));
    });

    zmq::socket_t& socket = socket_cache_->At(std::get<0>(t));
    zmq_util::send_msgs(std::move(msgs), &socket);
  }

  std::tuple<T, Ts...> Parse(const std::vector<std::string>& columns) const {
    return detail::parse_tuple<Pickler, T, Ts...>(columns);
  }

  void Receive(const std::tuple<T, Ts...>& t, std::size_t hash,
               int logical_time_inserted) {
    MergeCollectionTuple(t, hash, logical_time_inserted, &ts_);
  }

  std::map<std::tuple<T, Ts...>, CollectionTupleIds> Tick() {
    std::map<std::tuple<T, Ts...>, CollectionTupleIds> ts;
    std::swap(ts, ts_);
    return ts;
  }

 private:
  template <typename U>
  std::string ToString(const U& x) {
    return Pickler<typename std::decay<U>::type>().Dump(x);
  }

  const std::size_t id_;
  const std::string name_;
  const std::array<std::string, 1 + sizeof...(Ts)> column_names_;
  std::map<std::tuple<T, Ts...>, CollectionTupleIds> ts_;

  // Whenever a tuple with address `a` is added to a Channel, the socket
  // associated with `a` in `socket_cache_` is used to send the tuple.
  zmq_util::SocketCache* socket_cache_;

  FRIEND_TEST(Channel, TickClearsChannel);
};

}  // namespace fluent

#endif  // COLLETIONS_CHANNEL_H_
