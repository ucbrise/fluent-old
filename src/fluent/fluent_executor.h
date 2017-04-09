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
#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "common/tuple_util.h"
#include "common/type_list.h"
#include "fluent/channel.h"
#include "fluent/collection_util.h"
#include "fluent/network_state.h"
#include "fluent/periodic.h"
#include "fluent/periodic.h"
#include "fluent/rule.h"
#include "fluent/rule_tags.h"
#include "fluent/scratch.h"
#include "fluent/socket_cache.h"
#include "fluent/stdin.h"
#include "fluent/stdout.h"
#include "fluent/table.h"
#include "lineagedb/connection_config.h"
#include "ra/lineage_tuple.h"
#include "ra/lineaged_tuple.h"

namespace fluent {

// See below.
template <typename Collections, typename BootstrapRules, typename Rules,
          template <template <typename> class, template <typename> class>
          class LineageDbClient,
          template <typename> class Hash, template <typename> class ToSql>
class FluentExecutor;

// # Overview
// A FluentExecutor runs a Fluent program. You build up a Fluent program using
// a FluentBuilder and then use it to execute the program. See the README for
// an example.
//
// # Implementation
// - Every C in Cs is one of the following forms:
//     - Table<Us...>  - Scratch<Us...>  - Channel<Us...>
//     - Stdin         - Stdout          - Periodic
// - A FluentExecutor stores pointers to the collections in `collections_`.
// - A FluentExecutor stores bootstrap rules in `boostrap_rules_`.
// - A FluentExecutor stores rules in `rules_`.
// - sizeof...(Lhss) == sizeof...(RuleTags) == sizeof...(Rhss) and
//   - Lhss are pointers to collections,
//   - Every type in RuleTags is one of the rule tags in `rule_tags.h`, and
//   - Rhss are relational algebra expressions.
// - Bootstrap{Lhss,RuleTags,Rhss} work exactly like their non-Bootstrap
//   counterparts.
// - A FluentExecutor steals most of its guts from a FluentBuilder.
// - A FluentExecutor uses a LineageDbClient<Hash, Sql> object to record history
//   and lineage. If you don't want to record history or lineage, then use a
//   NoopClient. Otherwise use a PqxxClient.
template <typename... Cs, typename... BootstrapLhss,
          typename... BootstrapRuleTags, typename... BootstrapRhss,
          typename... Lhss, typename... RuleTags, typename... Rhss,
          template <template <typename> class, template <typename> class>
          class LineageDbClient,
          template <typename> class Hash, template <typename> class ToSql>
class FluentExecutor<
    TypeList<Cs...>,
    std::tuple<Rule<BootstrapLhss, BootstrapRuleTags, BootstrapRhss>...>,
    std::tuple<Rule<Lhss, RuleTags, Rhss>...>, LineageDbClient, Hash, ToSql> {
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
  using BootstrapRules =
      std::tuple<Rule<BootstrapLhss, BootstrapRuleTags, BootstrapRhss>...>;
  using Rules = std::tuple<Rule<Lhss, RuleTags, Rhss>...>;

  FluentExecutor(std::string name, std::size_t id,
                 std::tuple<std::unique_ptr<Cs>...> collections,
                 BootstrapRules bootstrap_rules,
                 std::map<std::string, Parser> parsers,
                 std::unique_ptr<NetworkState> network_state, Stdin* stdin,
                 std::vector<Periodic*> periodics,
                 std::unique_ptr<LineageDbClient<Hash, ToSql>> lineagedb_client,
                 Rules rules)
      : name_(std::move(name)),
        id_(id),
        collections_(std::move(collections)),
        bootstrap_rules_(std::move(bootstrap_rules)),
        parsers_(std::move(parsers)),
        network_state_(std::move(network_state)),
        stdin_(stdin),
        periodics_(periodics),
        lineagedb_client_(std::move(lineagedb_client)),
        rules_(rules) {
    // Initialize periodic timeouts. See the comment above `PeriodicTimeout`
    // below for more information.
    Periodic::time now = Periodic::clock::now();
    for (Periodic* p : periodics_) {
      timeout_queue_.push(PeriodicTimeout{now + p->Period(), p});
    }

    InitLineageDbClient();
  }

  // Get<I>() returns a const reference to the Ith collection.
  template <std::size_t I>
  const auto& Get() const {
    return *std::get<I>(collections_);
  }

  // DO_NOT_SUMIT(mwhittaker): Document.
  const LineageDbClient<Hash, ToSql>& GetLineageDbClient() {
    return *lineagedb_client_.get();
  }

  // Sequentially execute each registered bootstrap query and then invoke the
  // `Tick` method of every collection.
  void BootstrapTick() {
    if (sizeof...(BootstrapLhss) == 0) {
      return;
    }

    TupleIteri(bootstrap_rules_, [this](std::size_t rule_number, auto& rule) {
      ExecuteRule(rule_number, &rule);
    });
    time_++;
    TupleIter(collections_, [this](auto& c) { TickCollection(c.get()); });
  }

  // Sequentially execute each registered query and then invoke the `Tick`
  // method of every collection.
  void Tick() {
    TupleIteri(rules_, [this](std::size_t rule_number, auto& rule) {
      ExecuteRule(rule_number, &rule);
    });
    time_++;
    TupleIter(collections_, [this](auto& c) { TickCollection(c.get()); });
  }

  // (Potentially) block and receive messages sent by other Fluent nodes.
  // Receiving a message will insert it into the appropriate channel.
  void Receive() {
    time_++;

    std::vector<zmq::pollitem_t> pollitems = {
        {network_state_->socket, 0, ZMQ_POLLIN, 0}};
    if (stdin_ != nullptr) {
      pollitems.push_back(stdin_->Pollitem());
    }

    long timeout = GetPollTimoutInMicros();
    zmq_util::poll(timeout, &pollitems);

    // Read from the network.
    if (pollitems[0].revents & ZMQ_POLLIN) {
      // msgs[0] = dep node id
      // msgs[1] = dep collection name
      // msgs[2] = dep time
      // msgs[3] = tuple element 0
      // msgs[4] = tuple element 1
      // ...
      std::vector<zmq::message_t> msgs =
          zmq_util::recv_msgs(&network_state_->socket);

      std::vector<std::string> strings;
      for (std::size_t i = 3; i < msgs.size(); ++i) {
        strings.push_back(zmq_util::message_to_string(msgs[i]));
      }

      const std::size_t dep_node_id =
          FromString<std::size_t>(zmq_util::message_to_string(msgs[0]));
      const std::string channel_name = zmq_util::message_to_string(msgs[1]);
      const int dep_time =
          FromString<int>(zmq_util::message_to_string(msgs[2]));

      if (parsers_.find(channel_name) != std::end(parsers_)) {
        parsers_[channel_name](dep_node_id, dep_time, channel_name, strings,
                               time_);
      } else {
        LOG(WARNING) << "A message was received for a channel named "
                     << channel_name
                     << " but a parser for the channel was never registered.";
      }
    }

    // Read from stdin.
    if (stdin_ != nullptr && pollitems[1].revents & ZMQ_POLLIN) {
      lineagedb_client_->InsertTuple(stdin_->Name(), time_, stdin_->GetLine());
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
  // Initialize a node with the lineagedb database.
  void InitLineageDbClient() {
    // Nodes.
    lineagedb_client_->Init();

    // Collections.
    TupleIter(collections_, [this](const auto& collection) {
      // collection is of type `const unique_ptr<collection_type>&`.
      using collection_type = typename Unwrap<
          typename std::decay<decltype(collection)>::type>::type;
      AddCollection(collection,
                    typename CollectionTypes<collection_type>::type{});
    });

    // Rules.
    TupleIteri(
        bootstrap_rules_, [this](std::size_t i, const auto& bootstrap_rule) {
          lineagedb_client_->AddRule(i, true, bootstrap_rule.ToDebugString());
        });
    TupleIteri(rules_, [this](std::size_t i, const auto& rule) {
      lineagedb_client_->AddRule(i, false, rule.ToDebugString());
    });
  }

  // Register a collection with the lineagedb database.
  template <typename Collection, typename... Ts>
  void AddCollection(const std::unique_ptr<Collection>& c, TypeList<Ts...>) {
    lineagedb_client_->template AddCollection<Ts...>(
        c->Name(),
        CollectionTypeToString(GetCollectionType<Collection>::value));
  }

  // Tick a collection and insert the deleted tuples into the lineagedb
  // database.
  template <typename Collection>
  void TickCollection(Collection* c) {
    auto deleted = c->Tick();
    for (const auto& t : deleted) {
      lineagedb_client_->DeleteTuple(c->Name(), time_, t);
    }
  }

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
      lineagedb_client_->InsertTuple(timeout.periodic->Name(), time_,
                                     timeout.periodic->Tock());
      timeout.timeout = now + timeout.periodic->Period();
      timeout_queue_.push(timeout);
    }
  }

  // TODO(mwhittaker): Document.
  template <typename Lhs, typename Rhs, typename UpdateCollection>
  void ExecuteRule(int rule_number, Lhs* collection, const Rhs& ra,
                   bool inserted, UpdateCollection update_collection) {
    using column_types = typename Rhs::column_types;
    using tuple_type = typename TypeListToTuple<column_types>::type;
    Hash<tuple_type> hash;

    ranges::for_each(ra.ToPhysical().ToRange(), [&](const auto& lt) {
      std::size_t tuple_hash = hash(lt.tuple);
      for (const ra::LineageTuple& l : lt.lineage) {
        lineagedb_client_->AddDerivedLineage(l.collection, l.hash, rule_number,
                                             inserted, collection->Name(),
                                             tuple_hash, time_);
      }

      if (inserted) {
        lineagedb_client_->InsertTuple(collection->Name(), time_, lt.tuple);
        switch (GetCollectionType<Lhs>::value) {
          case CollectionType::CHANNEL:
          case CollectionType::STDOUT: {
            // When a tuple is inserted into a channel or stdout, it isn't
            // really inserted at all. Channels send their tuples away and
            // stdout just prints the message to the screen. Thus, we insert
            // and then immediately delete the tuple.
            lineagedb_client_->DeleteTuple(collection->Name(), time_, lt.tuple);
          }
          case CollectionType::TABLE:
          case CollectionType::SCRATCH:
          case CollectionType::STDIN:
          case CollectionType::PERIODIC: {
            // Do nothing.
          }
        }
      } else {
        lineagedb_client_->DeleteTuple(collection->Name(), time_, lt.tuple);
      }

      update_collection(*collection, std::set<tuple_type>{lt.tuple});
    });
  }

  template <typename... Ts, typename Rhs>
  void ExecuteRule(int rule_number, Channel<Ts...>* collection, MergeTag,
                   const Rhs& ra) {
    ExecuteRule(
        rule_number, collection, ra, true,
        [this](Channel<Ts...>& c, const std::set<std::tuple<Ts...>>& ts) {
          c.Merge(ts, time_);
        });
  }

  template <typename Lhs, typename Rhs>
  void ExecuteRule(int rule_number, Lhs* collection, MergeTag, const Rhs& ra) {
    ExecuteRule(rule_number, collection, ra, true, std::mem_fn(&Lhs::Merge));
  }

  template <typename Lhs, typename Rhs>
  void ExecuteRule(int rule_number, Lhs* collection, DeferredMergeTag,
                   const Rhs& ra) {
    ExecuteRule(rule_number, collection, ra, true,
                std::mem_fn(&Lhs::DeferredMerge));
  }

  template <typename Lhs, typename Rhs>
  void ExecuteRule(int rule_number, Lhs* collection, DeferredDeleteTag,
                   const Rhs& ra) {
    ExecuteRule(rule_number, collection, ra, false,
                std::mem_fn(&Lhs::DeferredDelete));
  }

  template <typename Lhs, typename RuleTag, typename Rhs>
  void ExecuteRule(std::size_t rule_number, Rule<Lhs, RuleTag, Rhs>* rule) {
    time_++;
    ExecuteRule(static_cast<int>(rule_number), CHECK_NOTNULL(rule->collection),
                rule->rule_tag, rule->ra);
  }

  // The logical time of the fluent program. The logical time begins at 0 and
  // is ticked before every rule execution and before every round of receiving
  // tuples. More concretely, grep for `time_++`.
  int time_ = 0;

  // See `FluentBuilder`.
  const std::string name_;
  const std::size_t id_;
  std::tuple<std::unique_ptr<Cs>...> collections_;
  BootstrapRules bootstrap_rules_;
  std::map<std::string, Parser> parsers_;
  std::unique_ptr<NetworkState> network_state_;
  Stdin* const stdin_;
  std::vector<Periodic*> periodics_;
  std::unique_ptr<LineageDbClient<Hash, ToSql>> lineagedb_client_;

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
