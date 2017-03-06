#ifndef FLUENT_FLUENT_EXECUTOR_H_
#define FLUENT_FLUENT_EXECUTOR_H_

#include <cstddef>

#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "common/type_list.h"
#include "fluent/channel.h"
#include "fluent/network_state.h"
#include "fluent/periodic.h"
#include "fluent/rule_tags.h"
#include "fluent/scratch.h"
#include "fluent/socket_cache.h"
#include "fluent/stdin.h"
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
// A few preliminaries.
//   - Every C in Cs is of the form
//       - Table<Us...>,
//       - Scratch<Us...>,
//       - Channel<Us...>,
//       - Stdin,
//       - Stdout, or
//       - Periodic.
//   - A FluentExecutor stores pointers to the collections in `collections_`.
//   - sizeof...(Lhss) == sizeof...(RuleTags) == sizeof...(Rhss) and
//     - Lhss are pointers to collections,
//     - Every type in RuleTags is one of the rule tags in `rule_tags.h`, and
//     - Rhss are relational algebra expressions.
//   - A FluentExecutor steals most of its guts from a FluentBuilder.
template <typename... Cs, typename... Lhss, typename... RuleTags,
          typename... Rhss>
class FluentExecutor<TypeList<Cs...>,
                     std::tuple<std::tuple<Lhss, RuleTags, Rhss>...>> {
 public:
  using Parser = std::function<void(const std::vector<std::string>& columns)>;

  FluentExecutor(std::tuple<std::unique_ptr<Cs>...> collections,
                 std::map<std::string, Parser> parsers,
                 std::unique_ptr<NetworkState> network_state, Stdin* stdin,
                 std::vector<Periodic*> periodics,
                 std::tuple<std::tuple<Lhss, RuleTags, Rhss>...> rules)
      : collections_(std::move(collections)),
        parsers_(std::move(parsers)),
        network_state_(std::move(network_state)),
        stdin_(stdin),
        periodics_(periodics),
        rules_(rules) {
    static_assert(sizeof...(Lhss) == sizeof...(RuleTags) &&
                      sizeof...(RuleTags) == sizeof...(Rhss),
                  "The ith entry of Lhss corresponds to the left-hand side of "
                  "the ith rule. The ith entry of RuleTags corresponds to the "
                  "type of the rule. The ith entry of Rhss corresponds to the "
                  "right-hand side of the ith rule. Thus, the sizes of Lhss, "
                  "RuleTags, and Rhss must be equal");
    LOG(INFO) << sizeof...(Lhss)
              << " rules registered with the FluentExecutor.";

    Periodic::time now = Periodic::clock::now();
    for (Periodic* p : periodics_) {
      timeout_queue_.push(PeriodicTimeout{now + p->Period(), p});
    }
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
    std::vector<zmq::pollitem_t> pollitems = {
        {network_state_->socket, 0, ZMQ_POLLIN, 0}};
    if (stdin_ != nullptr) {
      pollitems.push_back(stdin_->Pollitem());
    }

    long timeout = GetPollTimoutInMicros();
    zmq_util::poll(timeout, &pollitems);

    // Read from the network.
    if (pollitems[0].revents & ZMQ_POLLIN) {
      std::vector<zmq::message_t> msgs =
          zmq_util::recv_msgs(&network_state_->socket);
      std::vector<std::string> strings;
      for (std::size_t i = 1; i < msgs.size(); ++i) {
        strings.push_back(zmq_util::message_to_string(msgs[i]));
      }

      const std::string channel_name = zmq_util::message_to_string(msgs[0]);
      if (parsers_.find(channel_name) != std::end(parsers_)) {
        parsers_[channel_name](strings);
      } else {
        LOG(WARNING) << "A message was received for a channel named "
                     << channel_name
                     << " but a parser for the channel was never registered.";
      }
    }

    // Read from stdin.
    if (stdin_ != nullptr && pollitems[1].revents & ZMQ_POLLIN) {
      stdin_->GetLine();
    }

    // Trigger periodics.
    TockPeriodics();
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

 protected:
  // MutableGet<I>() returns a reference to the Ith collection. This should
  // only really ever be used for unit testing.
  template <std::size_t I>
  auto& MutableGet() const {
    return *std::get<I>(collections_);
  }

 private:
  // `GetPollTimoutInMicros` returns the minimum time (in microseconds) that we
  // need to wait before a PeriodicTimeout in `timeout_queue_` is ready. If
  // `timeout_queue_` is empty, then we return -1, which indicates that we
  // should wait forever.
  long GetPollTimoutInMicros() {
    if (timeout_queue_.size() == 0) {
      return -1;
    }

    // The `zmq_poll` API is a bit confusing. It says "zmq_poll() shall wait
    // timeout microseconds for an event to occur" but then also says "The
    // resolution of timeout is 1 millisecond". Then, if you look at `zmq.hpp`,
    // you see `poll` takes std::chrono::milliseconds [1]. After
    // experimentation, it definitely takes milliseconds.
    //
    // [1]: http://bit.ly/2n3SqEx
    CHECK(timeout_queue_.size() > 0);
    std::chrono::milliseconds timeout =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            timeout_queue_.top().timeout - Periodic::clock::now());
    return std::max<long>(0, timeout.count());
  }

  // Call `Tock` on every Periodic that's ready to be tocked. See the comment
  // on `PeriodicTimeout` down below for more information.
  void TockPeriodics() {
    Periodic::time now = Periodic::clock::now();
    while (timeout_queue_.size() != 0 && timeout_queue_.top().timeout <= now) {
      PeriodicTimeout timeout = timeout_queue_.top();
      timeout_queue_.pop();
      timeout.periodic->Tock();
      timeout.timeout = now + timeout.periodic->Period();
      timeout_queue_.push(timeout);
    }
  }

  // TODO(mwhittaker): Document.
  template <typename Lhs, typename Rhs>
  void ExecuteRule(Lhs* collection, MergeTag, const Rhs& ra) {
    collection->Merge(ra);
  }

  template <typename Lhs, typename Rhs>
  void ExecuteRule(Lhs* collection, DeferredMergeTag, const Rhs& ra) {
    collection->DeferredMerge(ra);
  }

  template <typename Lhs, typename Rhs>
  void ExecuteRule(Lhs* collection, DeferredDeleteTag, const Rhs& ra) {
    collection->DeferredDelete(ra);
  }

  // `ExecuteRules<0>` executes every rule in `rules_`.
  template <std::size_t I>
  typename std::enable_if<I == sizeof...(Rhss)>::type ExecuteRules() {}

  template <std::size_t I>
  typename std::enable_if<I != sizeof...(Rhss)>::type ExecuteRules() {
    if (I != sizeof...(Rhss)) {
      ExecuteRule(CHECK_NOTNULL(std::get<0>(std::get<I>(rules_))),
                  std::get<1>(std::get<I>(rules_)),
                  std::get<2>(std::get<I>(rules_)));
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

  // See `FluentBuilder`.
  std::tuple<std::unique_ptr<Cs>...> collections_;
  std::map<std::string, Parser> parsers_;
  std::unique_ptr<NetworkState> network_state_;
  Stdin* const stdin_;
  std::vector<Periodic*> periodics_;

  // A collection of rules (lhs, type, rhs) where
  //
  //   - `lhs` is a pointer to a collection,
  //   - `type` is an instance of one of the structs below, and
  //   - `rhs` is a relational algebra expression.
  //
  // See the class comment at the top of the file or see rule_tags.h for more
  // information.
  std::tuple<std::tuple<Lhss, RuleTags, Rhss>...> rules_;

  // When a FluentExecutor runs its Receive method, it has to worry about three
  // types of events triggering:
  //
  //   1. a fluent program can receive a message (via a channel) from a
  //      different fluent node,
  //   2. a fluent program can read a message from stdin, or
  //   3. a Periodic in a fluent program can trigger.
  //
  // To simultaneously wait for the first two events, we perform a zmq::poll.
  // To wait for the last type of event, we have to set the timeout to
  // zmq::poll just right. Here's how that works.
  //
  // We maintain a priority queue `timeout_queue_` of PeriodicTimeout structs:
  // one for each Periodic in `periodics_`. Each PeriodicTimeout records the
  // time at which it should be triggered and the associated Periodic. These
  // PeriodicTimeouts are ordered by increasing timeout using the
  // PeriodicTimeoutCompare struct below.
  //
  // For example, imagine we have 3 periodics:
  //
  //   - x with a period of 2,
  //   - y with a period of 4, and
  //   - z with a period of 16.
  //
  // Here is the contents of `timeout_queue_` at time 0.
  //
  //           +--------------+--------------+--------------+
  //   time  0 | timeout:  2  | timeout:  4  | timeout:  16 |
  //           | periodic: &x | periodic: &y | periodic: &z |
  //           +--------------+--------------+--------------+
  //
  // Then, we set our poll timeout to wake up just in time for the first
  // timeout. In our example, our poll timeout would be 2. Every time, the poll
  // is triggered, we iterate through the PeriodicTimeouts that should be
  // triggered, trigger them, update their timeout, and then push them back
  // onto the queue. Here's what the state of the queue looks like over time:
  //
  //           +--------------+--------------+--------------+
  //   time  2 | timeout:  4  | timeout:  4  | timeout:  16 |
  //           | periodic: &x | periodic: &y | periodic: &z |
  //           +--------------+--------------+--------------+
  //           +--------------+--------------+--------------+
  //   time  4 | timeout:  6  | timeout:  8  | timeout:  16 |
  //           | periodic: &x | periodic: &y | periodic: &z |
  //           +--------------+--------------+--------------+
  //           +--------------+--------------+--------------+
  //   time  6 | timeout:  8  | timeout:  8  | timeout:  16 |
  //           | periodic: &x | periodic: &y | periodic: &z |
  //           +--------------+--------------+--------------+
  //   ...
  //           +--------------+--------------+--------------+
  //   time 16 | timeout:  18 | timeout:  20 | timeout:  32 |
  //           | periodic: &x | periodic: &y | periodic: &z |
  //           +--------------+--------------+--------------+
  struct PeriodicTimeout {
    Periodic::time timeout;
    Periodic* periodic;
  };

  // See `PeriodicTimeout`.
  struct PeriodicTimeoutCompare {
    bool operator()(const PeriodicTimeout& lhs, const PeriodicTimeout& rhs) {
      return lhs.timeout < rhs.timeout;
    }
  };

  // See `PeriodicTimeout`.
  std::priority_queue<PeriodicTimeout, std::vector<PeriodicTimeout>,
                      PeriodicTimeoutCompare>
      timeout_queue_;

  FRIEND_TEST(FluentExecutor, SimpleCommunication);
};

}  // namespace fluent

#endif  // FLUENT_FLUENT_EXECUTOR_H_
