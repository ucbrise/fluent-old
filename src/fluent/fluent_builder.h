#ifndef FLUENT_FLUENT_BUILDER_H_
#define FLUENT_FLUENT_BUILDER_H_

#include <cstddef>

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "zmq.hpp"

#include "common/type_list.h"
#include "fluent/channel.h"
#include "fluent/fluent_executor.h"
#include "fluent/network_state.h"
#include "fluent/periodic.h"
#include "fluent/scratch.h"
#include "fluent/socket_cache.h"
#include "fluent/stdin.h"
#include "fluent/stdout.h"
#include "fluent/table.h"
#include "postgres/client.h"

namespace fluent {

template <typename Collections, typename BootstrapRules>
class FluentBuilder;

// # Overview
// A FluentBuilder is a class that you can use to build up a FluentExecutor. It
// is best explained through an example.
//
//   // We'll use this to bootstrap our fluent program later.
//   std::vector<std::tuple<int, char, float>> tuples = {
//     {1, 'a', 1.0}, {2, 'b', 2.0}, {3, 'c', 3.0}, {4, 'd', 4.0}
//   }
//
//   // Each FluentExecutor contains a socket which listens for messages from
//   // other Fluent nodes. Here, our FluentExecutor will listen on
//   // "tcp://*:8000", so we pass that address to the FluentBuilder.
//   const std::string address = "tcp://*:8000";
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
// # Implementation
// Each FluentBuilder maintains two key fields (in addition to a bunch of other
// less important fields): `collections_` and `boostrap_rules_`.
//
// `collections_` contains pointers to every collection registered with this
// FluentBuilder. For example, given the following FluentBuilder:
//
//   auto f = fluent(address)
//     .table<int, char, float>("t1")
//     .table<float, int>("t2")
//     .scratch<int, int, float>("s")
//     .channel<std::string, float, char>("c")
//
// `collections_` will have the following type:
//
//   std::tuple<
//     std::unique_ptr<Table<int, char, float>>,          // t1
//     std::unique_ptr<Table<float, int>>,                // t2
//     std::unique_ptr<Scratch<int, int, float>>,         // s
//     std::unique_ptr<Channel<std::string, float, char>> // c
//   >
//
// `boostrap_rules_` is a collection of rules of the form (lhs, type, rhs)
// where
//
//   - `lhs` is a pointer to a collection,
//   - `type` is an instance of one of the structs below, and
//   - `rhs` is a relational algebra expression.
//
// The Ith rule has type
//
//   std::tuple<BootstrapLhss[I], BootstrapRuleTags[I], BootstrapRhss[I]>
//
// Whenever a user calls a function like `.table`, `.scratch`, or `.channel`,
// we return a brand new FluentBuilder with the newly registered collection
// appended to `collections_`. Whenever a user calls `.RegisterBootstrapRules`,
// we return a brand new FluentBuilder with the newly registered rules
// replacing `boostrap_rules_`.
template <typename... Cs, typename... BootstrapLhss,
          typename... BootstrapRuleTags, typename... BootstrapRhss>
class FluentBuilder<TypeList<Cs...>,
                    std::tuple<std::tuple<BootstrapLhss, BootstrapRuleTags,
                                          BootstrapRhss>...>> {
  static_assert(sizeof...(BootstrapLhss) == sizeof...(BootstrapRuleTags) &&
                    sizeof...(BootstrapRuleTags) == sizeof...(BootstrapRhss),
                "The ith entry of BootstrapLhss corresponds to the left-hand "
                "side of the ith bootstrap rule. The ith entry of "
                "BootstrapRuleTags corresponds to the type of the bootstrap "
                "rule. The ith entry of BootstrapRhss corresponds to the "
                "right-hand side of the ith bootstrap rule. Thus, the sizes of "
                "BootstrapLhss, BootstrapRuleTags, and BootstrapRhss must be "
                "equal");

 public:
  using BootstrapRules = std::tuple<
      std::tuple<BootstrapLhss, BootstrapRuleTags, BootstrapRhss>...>;
  using Parser = std::function<void(const std::vector<std::string>& columns)>;

  // Create a table, scratch, channel, stdin, stdout, or periodic. Note the
  // `&&` at the end of each declaration. This means that these methods can
  // only be invoked on an rvalue-reference, which is necessary since the
  // methods move their contents.

  template <typename... Us>
  FluentBuilder<TypeList<Cs..., Table<Us...>>, BootstrapRules> table(
      const std::string& name) && {
    LOG(INFO) << "Adding a table named " << name << ".";
    return AddCollection(std::make_unique<Table<Us...>>(name));
  }

  template <typename... Us>
  FluentBuilder<TypeList<Cs..., Scratch<Us...>>, BootstrapRules> scratch(
      const std::string& name) && {
    LOG(INFO) << "Adding a scratch named " << name << ".";
    return AddCollection(std::make_unique<Scratch<Us...>>(name));
  }

  template <typename... Us>
  FluentBuilder<TypeList<Cs..., Channel<Us...>>, BootstrapRules> channel(
      const std::string& name) && {
    LOG(INFO) << "Adding a channel named " << name << ".";
    auto c =
        std::make_unique<Channel<Us...>>(name, &network_state_->socket_cache);
    CHECK(parsers_.find(c->Name()) == parsers_.end())
        << "The channel name '" << c->Name()
        << "' is used multiple times. Channel names must be unique.";
    parsers_.insert(std::make_pair(c->Name(), c->GetParser()));
    return AddCollection(std::move(c));
  }

  FluentBuilder<TypeList<Cs..., Stdin>, BootstrapRules> stdin() && {
    LOG(INFO) << "Adding stdin.";
    auto stdin = std::make_unique<Stdin>();
    stdin_ = stdin.get();
    return AddCollection(std::move(stdin));
  }

  FluentBuilder<TypeList<Cs..., Stdout>, BootstrapRules> stdout() && {
    LOG(INFO) << "Adding stdout.";
    return AddCollection(std::make_unique<Stdout>());
  }

  FluentBuilder<TypeList<Cs..., Periodic>, BootstrapRules> periodic(
      const std::string& name, const Periodic::period& period) && {
    LOG(INFO) << "Adding Periodic named " << name << ".";
    auto p = std::make_unique<Periodic>(name, period);
    periodics_.push_back(p.get());
    return AddCollection(std::move(p));
  }

  // See `RegisterRules`
  template <typename F>
  FluentBuilder<TypeList<Cs...>, typename std::result_of<F(Cs&...)>::type>
  RegisterBootstrapRules(const F& f) && {
    static_assert(sizeof...(BootstrapLhss) == 0,
                  "You are registering bootstrap rules with a FluentBuilder "
                  "that already has bootstrap rules registered with it. This "
                  "is disallowed.");
    return RegisterBootstrapRulesImpl(
        f, std::make_index_sequence<sizeof...(Cs)>());
  }

  // Recall the example from above:
  //
  //   auto f = fluent(address)
  //     .table<int, char, float>("t1")
  //     .table<float, int>("t2")
  //     .scratch<int, int, float>("s")
  //     .channel<std::string, float, char>("c")
  //     .RegisterRules([](auto& t1, auto& t2, auto& s, auto& c) {
  //       return std::make_tuple(t2 <= t1.iterable | ra::project<2, 0>());
  //     });
  //
  // `f` is a function which takes in a reference to each of the collections
  // registered with the FluentBuilder in the order they are registered. It
  // outputs a tuple of rules where each rule is a pair of the form (lhs, rhs)
  // where
  //   - lhs is a pointer to a collection in the FluentBuilder, and
  //   - rhs is a relational algebra expression.
  // Note that `t <= ra` is syntactic sugar for `std::make_pair(t, ra)`
  // implemented by Collection's `<=` operator. `RegisterRules` will execute
  // `f` to generate the rules and use them to construct a `FluentExecutor`.
  template <typename F>
  FluentExecutor<TypeList<Cs...>, BootstrapRules,
                 typename std::result_of<F(Cs&...)>::type>
  RegisterRules(const F& f) && {
    return RegisterRulesImpl(f, std::make_index_sequence<sizeof...(Cs)>());
  }

 private:
  // Constructs an empty FluentBuilder. Note that this constructor should
  // only be called when Cs is empty (i.e. sizeof...(Cs) == 0). This private
  // constructor is used primarily by the `fluent` function down below.
  FluentBuilder(const std::string& address, zmq::context_t* context,
                postgres::Client* postgres_client)
      : network_state_(std::make_unique<NetworkState>(address, context)),
        stdin_(nullptr),
        postgres_client_(postgres_client) {
    static_assert(sizeof...(Cs) == 0,
                  "The FluentBuilder(const std::string& address, "
                  "zmq::context_t* const context) constructor should only be "
                  "called when Cs is empty.");
  }

  // Moves the guts of one FluentBuilder into another.
  FluentBuilder(std::tuple<std::unique_ptr<Cs>...> collections,
                BootstrapRules boostrap_rules,
                std::map<std::string, Parser> parsers,
                std::unique_ptr<NetworkState> network_state, Stdin* stdin,
                std::vector<Periodic*> periodics,
                postgres::Client* postgres_client)
      : collections_(std::move(collections)),
        boostrap_rules_(std::move(boostrap_rules)),
        parsers_(std::move(parsers)),
        network_state_(std::move(network_state)),
        stdin_(stdin),
        periodics_(std::move(periodics)),
        postgres_client_(postgres_client) {}

  FluentBuilder(FluentBuilder&&) = default;
  FluentBuilder& operator=(FluentBuilder&&) = default;
  DISALLOW_COPY_AND_ASSIGN(FluentBuilder);

  // Return a new FluentBuilder with `c` appended to `collections`.
  template <typename C>
  FluentBuilder<TypeList<Cs..., C>, BootstrapRules> AddCollection(
      std::unique_ptr<C> c) {
    std::tuple<std::unique_ptr<Cs>..., std::unique_ptr<C>> collections =
        std::tuple_cat(std::move(collections_), std::make_tuple(std::move(c)));
    return {std::move(collections),
            std::move(boostrap_rules_),
            std::move(parsers_),
            std::move(network_state_),
            stdin_,
            std::move(periodics_),
            postgres_client_};
  }

  // See `RegisterBootstrapRules`.
  template <typename F, std::size_t... Is>
  FluentBuilder<TypeList<Cs...>, typename std::result_of<F(Cs&...)>::type>
  RegisterBootstrapRulesImpl(const F& f, std::index_sequence<Is...>) {
    auto boostrap_rules = f(*std::get<Is>(collections_)...);
    return {std::move(collections_),
            std::move(boostrap_rules),
            std::move(parsers_),
            std::move(network_state_),
            stdin_,
            std::move(periodics_),
            postgres_client_};
  }

  // See `RegisterRules`.
  template <typename F, std::size_t... Is>
  FluentExecutor<TypeList<Cs...>, BootstrapRules,
                 typename std::result_of<F(Cs&...)>::type>
  RegisterRulesImpl(const F& f, std::index_sequence<Is...>) {
    auto relalgs = f(*std::get<Is>(collections_)...);
    return {std::move(collections_),
            std::move(boostrap_rules_),
            std::move(parsers_),
            std::move(network_state_),
            stdin_,
            std::move(periodics_),
            std::move(postgres_client_),
            std::move(relalgs)};
  }

  // See class documentation above.
  //
  // TODO(mwhittaker): Right now, I made things unique_ptr because I was
  // paranoid that as data was being moved from one FluentBuilder to
  // another, pointers to fields would be invalidated. Maybe we don't need the
  // unique_ptr, but I'd need to think harder about it.
  std::tuple<std::unique_ptr<Cs>...> collections_;

  // See class documentation above.
  BootstrapRules boostrap_rules_;

  // `parsers_`  maps channel names to parsing functions that can parse a
  // packet (represented as a vector of strings) into a tuple and insert it
  // into the appropriate channel. For example, imagine we had a
  // `Channel<std::string, int, float>` channel named `"c"`. `parsers_` would
  // contain an entry for key `"c"` and `parsers_["c"]({"foo", "1", "2.0"})`
  // would insert the tuple ("foo", 1, 2.0) into `"c"`.
  std::map<std::string, Parser> parsers_;

  // Each fluent program sends tuples to other fluent nodes over the network
  // and receives tuples from other fluent nodes over the network. This is the
  // networking state needed to perform that networking.
  std::unique_ptr<NetworkState> network_state_;

  // This pointer, if not null, points into the Stdin object inside of
  // `collections_`. If it is null, then there is no Stdin object in
  // `collections_`.
  Stdin* stdin_;

  // A vector of the Periodics inside of `collections_`, in no particular
  // order.
  std::vector<Periodic*> periodics_;

  // DO_NOT_SUBMIT(mwhittaker): Document.
  postgres::Client* const postgres_client_;

  // All FluentBuilders are friends of one another.
  template <typename Collections, typename BootstrapRules>
  friend class FluentBuilder;

  // The `fluent` function is used to construct a FluentBuilder without any
  // collections. Recall that looks something like this:
  //
  //   auto f = fluent("tcp://*:8000");
  //     .table<int, char, float>("t")
  //     .scratch<int, int, float>("s")
  //     // and so on...
  friend FluentBuilder<TypeList<>, std::tuple<>> fluent(
      const std::string& address, zmq::context_t* context,
      postgres::Client* postgres_client);
};

// Create an empty FluentBuilder listening on ZeroMQ address `address` using
// the ZeroMQ context `context`.
inline FluentBuilder<TypeList<>, std::tuple<>> fluent(
    const std::string& address, zmq::context_t* context,
    postgres::Client* postgres_client) {
  return {address, context, postgres_client};
}

}  // namespace fluent

#endif  // FLUENT_FLUENT_BUILDER_H_
