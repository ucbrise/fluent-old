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
#include "fluent/base_lattice.h"
#include "fluent/bool_lattice.h"
#include "fluent/channel.h"
#include "fluent/max_lattice.h"
#include "fluent/map_lattice.h"
#include "fluent/min_lattice.h"
#include "fluent/network_state.h"
#include "fluent/periodic.h"
#include "fluent/rule_tags.h"
#include "fluent/scratch.h"
#include "fluent/socket_cache.h"
#include "fluent/stdin.h"
#include "fluent/table.h"

namespace fluent {

template <typename Collections, typename BootstrapRules, typename Rules>
class FluentExecutor;

// # Overview
// A FluentExecutor runs a Fluent program. You build up a Fluent program using
// a FluentBuilder and then use it to execute the program. It is best
// explained through an example.
//
//   // This FluentExecutor will have four collections:
//   //   - a 3-column table named "t1" with types [int, char, float]
//   //   - a 2-column table named "t2" with types [float, int]
//   //   - a 3-column scratch named "s" with types [int, int, float]
//   //   - a 3-column channel named "c" with types [std::string, float, char]
//   // The FluentExecutor will also have two rules:
//   //   - The first is a bootstrap rule (registered with
//   //     RegisterBootstrapRules) that will move the contents of `tuples`
//   //     into `t1`. This rule will be executed exactly once at the beginning
//   //     of the fluent program.
//   //   - The second rule projects out the third and first columns of t1 and
//   //     puts them into t2. It will be run every tick of the program.
//   auto f = fluent(address)
//     .table<int, char, float>("t1")
//     .table<float, int>("t2")
//     .scratch<int, int, float>("s")
//     .channel<std::string, float, char>("c")
//     .RegisterBootstrapRules([&](auto& t1, auto& t2, auto& s, auto& c) {
//       return std::make_tuple(t1 <= ra::make_iterable(&tuples));
//     });
//     .RegisterRules([](auto& t1, auto& t2, auto& s, auto& c) {
//       return std::make_tuple(t2 <= t1.iterable | ra::project<2, 0>());
//     });
//
//   // Calling `f.BootstrapTick()` will run the bootstrap rules. Every rule
//   // registered with `RegisterBootstrapRules` above will be called.
//   // Afterwards, every collection is "ticked". For example, scratches and
//   // channels are cleared.
//   f.BootstrapTick();
//
//   // Calling `f.Tick()` will run the rules in exactly the same way
//   // `f.BootstrapTick()` ran the bootstrap rules.
//   f.Tick();
//
//   // Calling `f.Run()` will run a Fluent program. The program will call
//   // `f.BootstrapTick()` and then repeatedly call `f.Tick()` and then wait
//   // for messages to arrive from other Fluent nodes, populating channels
//   // appropriately.
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
//   - Bootstrap{Lhss,RuleTags,Rhss} work exactly like their non-Bootstrap
//     counterparts.
//   - A FluentExecutor steals most of its guts from a FluentBuilder.
template <typename... Cs, typename... BootstrapLhss,
          typename... BootstrapRuleTags, typename... BootstrapRhss,
          typename... Lhss, typename... RuleTags, typename... Rhss>
class FluentExecutor<
    TypeList<Cs...>,
    std::tuple<std::tuple<BootstrapLhss, BootstrapRuleTags, BootstrapRhss>...>,
    std::tuple<std::tuple<Lhss, RuleTags, Rhss>...>> {
  static_assert(sizeof...(BootstrapLhss) == sizeof...(BootstrapRuleTags) &&
                    sizeof...(BootstrapRuleTags) == sizeof...(BootstrapRhss),
                "The ith entry of BootstrapLhss corresponds to the left-hand "
                "side of the ith bootstrap rule. The ith entry of "
                "BootstrapRuleTags corresponds to the type of the bootstrap "
                "rule. The ith entry of BootstrapRhss corresponds to the "
                "right-hand side of the ith bootstrap rule. Thus, the sizes of "
                "BootstrapLhss, BootstrapRuleTags, and BootstrapRhss must be "
                "equal");

  static_assert(sizeof...(Lhss) == sizeof...(RuleTags) &&
                    sizeof...(RuleTags) == sizeof...(Rhss),
                "The ith entry of Lhss corresponds to the left-hand side of "
                "the ith rule. The ith entry of RuleTags corresponds to the "
                "type of the rule. The ith entry of Rhss corresponds to the "
                "right-hand side of the ith rule. Thus, the sizes of Lhss, "
                "RuleTags, and Rhss must be equal");

 public:
  using BootstrapRules = std::tuple<
      std::tuple<BootstrapLhss, BootstrapRuleTags, BootstrapRhss>...>;
  using Rules = std::tuple<std::tuple<Lhss, RuleTags, Rhss>...>;
  using Parser = std::function<void(const std::vector<std::string>& columns)>;

  FluentExecutor(std::tuple<std::unique_ptr<Cs>...> collections,
                 BootstrapRules bootstrap_rules,
                 std::map<std::string, Parser> parsers,
                 std::unique_ptr<NetworkState> network_state, Stdin* stdin,
                 std::vector<Periodic*> periodics, Rules rules)
      : collections_(std::move(collections)),
        bootstrap_rules_(std::move(bootstrap_rules)),
        parsers_(std::move(parsers)),
        network_state_(std::move(network_state)),
        stdin_(stdin),
        periodics_(periodics),
        rules_(rules) {
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

  // Sequentially execute each registered bootstrap query and then invoke the
  // `Tick` method of every collection.
  void BootstrapTick() {
    if (sizeof...(BootstrapLhss) != 0) {
      ExecuteBootstrapRules<0>();
      TickCollections<0>();
    }
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
      bool res;
      while (true) {
        std::vector<zmq::message_t> msgs;
        if ((res = zmq_util::recv_msgs(&network_state_->socket, msgs)) == false) break;
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
    BootstrapTick();
    while (true) {
      Receive();
      Tick();
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

  // `ExecuteBootstrapRules<0>` executes every rule in `bootstrap_rules_`.
  template <std::size_t I>
  typename std::enable_if<I == sizeof...(BootstrapRhss)>::type
  ExecuteBootstrapRules() {}

  template <std::size_t I>
  typename std::enable_if<I != sizeof...(BootstrapRhss)>::type
  ExecuteBootstrapRules() {
    if (I != sizeof...(BootstrapRhss)) {
      ExecuteRule(CHECK_NOTNULL(std::get<0>(std::get<I>(bootstrap_rules_))),
                  std::get<1>(std::get<I>(bootstrap_rules_)),
                  std::get<2>(std::get<I>(bootstrap_rules_)));
      ExecuteBootstrapRules<I + 1>();
    }
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
  BootstrapRules bootstrap_rules_;
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
  Rules rules_;

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
