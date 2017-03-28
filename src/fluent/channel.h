#ifndef FLUENT_CHANNEL_H_
#define FLUENT_CHANNEL_H_

#include <cstddef>

#include <algorithm>
#include <set>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/type_traits.h"
#include "fluent/rule_tags.h"
#include "fluent/serialization.h"
#include "fluent/socket_cache.h"
#include "ra/iterable.h"
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
class Channel {
  static_assert(std::is_same<std::string, T>::value,
                "The first column of a channel must be a string specifying a "
                "ZeroMQ address (e.g. tcp://localhost:9999).");

 public:
  Channel(std::string name, SocketCache* socket_cache)
      : name_(std::move(name)), socket_cache_(socket_cache) {}

  const std::string& Name() const { return name_; }

  const std::set<std::tuple<T, Ts...>>& Get() const { return ts_; }

  ra::Iterable<std::set<std::tuple<T, Ts...>>> Iterable() const {
    return ra::make_iterable(&ts_);
  }

  // Merge assumes a `std::string ToString(const U&)` function exists for `U`
  // in `T, Ts...`.
  template <typename RA>
  void Merge(const RA& ra) {
    static_assert(!IsSet<typename std::decay<RA>::type>::value, "");
    static_assert(!IsVector<typename std::decay<RA>::type>::value, "");
    auto physical = ra.ToPhysical();
    auto rng = physical.ToRange();
    MergeImpl(ranges::begin(rng), ranges::end(rng),
              std::make_index_sequence<sizeof...(Ts) + 1>());
  }

  void Merge(const std::set<std::tuple<T, Ts...>>& ts) {
    MergeImpl(std::begin(ts), std::end(ts),
              std::make_index_sequence<sizeof...(Ts) + 1>());
  }

  std::set<std::tuple<T, Ts...>> Tick() {
    std::set<std::tuple<T, Ts...>> ts;
    std::swap(ts, ts_);
    return ts;
  }

  // `Collection<T1, ..., Tn>.GetParser()(columns)` parses a vector of `n`
  // strings into a tuple of type `std::tuple<T1, ..., Tn>` and inserts it into
  // the collection. The ith element of the tuple is parsed using a global `Ti
  // FromString<Ti>(const std::string&)` function which is assumed to exists
  // (see `serialization.h` for more information).
  template <typename F>
  std::function<void(std::size_t, const std::vector<std::string>& columns)>
  GetParser(F f) {
    return
        [this, f](std::size_t time, const std::vector<std::string>& columns) {
          const auto t = detail::parse_tuple<T, Ts...>(columns);
          f(time, t);
          ts_.insert(t);
        };
  }

 private:
  template <typename Iterator, typename Sentinel, std::size_t... Is>
  void MergeImpl(Iterator iter, Sentinel end, std::index_sequence<Is...>) {
    for (; iter != end; ++iter) {
      VLOG(1) << "Channel " << this->Name() << " sending tuple to "
              << std::get<0>(*iter) << ".";

      std::vector<std::string> strings = {ToString(std::get<Is>(*iter))...};
      std::vector<zmq::message_t> msgs;
      msgs.push_back(zmq_util::string_to_message(this->Name()));
      for (const std::string& s : strings) {
        msgs.push_back(zmq_util::string_to_message(s));
      }

      zmq::socket_t& socket = socket_cache_->At(std::get<0>(*iter));
      zmq_util::send_msgs(std::move(msgs), &socket);
    }
  }

  const std::string name_;
  std::set<std::tuple<T, Ts...>> ts_;

  // Whenever a tuple with address `a` is added to a Channel, the socket
  // associated with `a` in `socket_cache_` is used to send the tuple.
  SocketCache* socket_cache_;

  FRIEND_TEST(Channel, TickClearsChannel);
};

}  // namespace fluent

#endif  // FLUENT_CHANNEL_H_
