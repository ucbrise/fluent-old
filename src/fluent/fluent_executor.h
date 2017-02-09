#ifndef FLUENT_FLUENT_EXECUTOR_H_
#define FLUENT_FLUENT_EXECUTOR_H_

#include <cstddef>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "fluent/channel.h"
#include "fluent/network_state.h"
#include "fluent/scratch.h"
#include "fluent/socket_cache.h"
#include "fluent/table.h"

namespace fluent {

template <typename Collections, typename Rules>
class FluentExecutor;

// # Overview
// A FluentExecutor runs a Fluent program. You build up a Fluent program using
// a FluentBuilder and then use it to execute the program. It is best
// explained through an example.
//
//   // This Fluent program will have four collections:
//   //   - a 3-column table named "t1" with types [int, char, float]
//   //   - a 2-column table named "t2" with types [float, int]
//   //   - a 3-column scratch named "s" with types [int, int, float]
//   //   - a 3-column channel named "c" with types [std::string, float, char]
//   // and a single rule which projects out the third and first columns of t1
//   // and puts them into t2.
//   auto f = fluent("tcp://*:8000")
//     .table<int, char, float>("t1")
//     .table<float, int>("t2")
//     .scratch<int, int, float>("s")
//     .channel<std::string, float, char>("c")
//     .RegisterRules([](auto& t1, auto& t2, auto& s, auto& c) {
//       return std::make_tuple(t2 <= t1.iterable | ra::project<2, 0>());
//     });
//
//   // Calling `f.Tick()` will run a single round of execution. Every rule
//   // registered with `RegisterRules` above will be called. After the round,
//   // scratches and channels are cleared. In our example, calling tick will
//   // move the projected contents of `t2` into `t1`. `t2` is initially empty,
//   // so this won't do anything.
//   f.Tick();
//
//   // Calling `f.Run()` will run a Fluent program. The program will
//   // repeatedly call `f.Tick()` and then wait for messages to arrive from
//   // other Fluent nodes, populating channels appropriately.
//   f.Run();
//
// # Implementation
// A few notes.
//   - Every C in Cs is of the form Table<Us...>, Scratch<Us...>, or
//     Channel<Us...>. A FluentExecutor stores pointers to the collections in
//     `collections_`.
//   - sizeof...(Lhss) == sizeof...(Rhss). Lhss are pointers to collections and
//     represent the left-hand sides of rules. Rhss are relational algebra
//     expressions and represent the right-hand sides of rules. Rules are
//     stored in `rules_`. See `FluentBuilder` for an example of how these
//     rules are built.
//   - A FluentExecutor steals most of its guts from a FluentBuilder.
template <typename... Cs, typename... Lhss, typename... Rhss>
class FluentExecutor<std::tuple<std::unique_ptr<Cs>...>,
                     std::tuple<std::pair<Lhss, Rhss>...>> {
 public:
  using Parser = std::function<void(const std::vector<std::string>& columns)>;

  FluentExecutor(std::tuple<std::unique_ptr<Cs>...> collections,
                 std::map<std::string, Parser> parsers,
                 std::unique_ptr<NetworkState> network_state,
                 std::tuple<std::pair<Lhss, Rhss>...> rules)
      : collections_(std::move(collections)),
        parsers_(std::move(parsers)),
        network_state_(std::move(network_state)),
        rules_(rules) {
    static_assert(sizeof...(Lhss) == sizeof...(Rhss),
                  "The ith entry of Lhss corresponds to the left-hand side of "
                  "the ith rule. The ith entry of Rhss corresponds to the "
                  "right-hand side of the ith rule. Thus, the sizes of Lhss "
                  "and Rhss must be equal");
    LOG(INFO) << sizeof...(Lhss) << " rules registered.";
  }

  // Get<I>() returns a const reference to the Ith collection.
  template <std::size_t I>
  const auto& Get() const {
    return *std::get<I>(collections_);
  }

  // Sequentially execute each registered query and then invoke the `Tick`
  // method of every collection.
  void Tick() {
    ExecuteRules<0>();
    TickCollections<0>();
  }

  // (Potentially) block and receive messages sent by other Fluent nodes.
  // Receiving a message will insert it into the appropriate channel.
  void Receive() {
    std::vector<zmq::message_t> msgs =
        zmq_util::recv_msgs(&network_state_->socket);
    std::vector<std::string> strings;
    for (std::size_t i = 1; i < msgs.size(); ++i) {
      strings.push_back(zmq_util::message_to_string(msgs[i]));
    }
    // TODO(mwhittaker): Check to see if zmq_util::message_to_string(msgs[0])
    // is in parsers_, logging a warning or something if it is not.
    parsers_[zmq_util::message_to_string(msgs[0])](strings);
  }

  // Runs a fluent program.
  // TODO(mwhittaker): Figure out if it should be Receive() then Tick() or
  // Tick() then Receive()?
  void Run() {
    while (true) {
      Tick();
      Receive();
    }
  }

 private:
  // MutableGet<I>() returns a reference to the Ith collection. This should only
  // really ever be used for unit testing.
  template <std::size_t I>
  auto& MutableGet() const {
    return *std::get<I>(collections_);
  }

  // `ExecuteRules<0>` executes every rule in `rules_`.
  template <std::size_t I>
  typename std::enable_if<I == sizeof...(Rhss)>::type ExecuteRules() {}

  template <std::size_t I>
  typename std::enable_if<I != sizeof...(Rhss)>::type ExecuteRules() {
    if (I != sizeof...(Rhss)) {
      CHECK_NOTNULL(std::get<I>(rules_).first)
          ->AddRelalg(std::get<I>(rules_).second);
      ExecuteRules<I + 1>();
    }
  }

  // `TickCollections<0>` calls `Tick` on every collection in `collection_`.
  template <std::size_t I>
  typename std::enable_if<I == sizeof...(Cs)>::type TickCollections() {}

  template <std::size_t I>
  typename std::enable_if<I != sizeof...(Cs)>::type TickCollections() {
    if (I != sizeof...(Cs)) {
      std::get<I>(collections_)->Tick();
      TickCollections<I + 1>();
    }
  }

  std::tuple<std::unique_ptr<Cs>...> collections_;
  std::map<std::string, Parser> parsers_;
  std::unique_ptr<NetworkState> network_state_;
  std::tuple<std::pair<Lhss, Rhss>...> rules_;

  FRIEND_TEST(FluentExecutor, SimpleCommunication);
};

}  // namespace fluent

#endif  // FLUENT_FLUENT_EXECUTOR_H_
