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

namespace fluent {

// # Overview
// A FluentBuilder is a class that you can use to build up a FluentExecutor. It
// is best explained through an example.
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
//   // and a single rule which projects out the third and first columns of t1
//   // and puts them into t2.
//   auto f = fluent(address)
//     .table<int, char, float>("t1")
//     .table<float, int>("t2")
//     .scratch<int, int, float>("s")
//     .channel<std::string, float, char>("c")
//     .RegisterRules([](auto& t1, auto& t2, auto& s, auto& c) {
//       return std::make_tuple(t2 <= t1.iterable | ra::project<2, 0>());
//     });
//
// # Implementation details
// In order to support the nice `.table`, `.scratch`, and `.channel` syntax,
// FluentBuilder requires quite a bit of metaprogramming. Here, we briefly
// explain FluentBuilder's implementation.
//
// A FluentBuilder<T1, ..., Tn> corresponds to a Fluent program with n
// collections.  Each Ti is either a Table<Us...>, Scratch<Us...>, or
// Channel<Us...>.  Pointers to the collections are stored `collections_` which
// is of type `std::tuple<std::unique_ptr<T1>, ..., std::unique_ptr<Tn>>`.
//
// Imagine we call `f.table<int>("foo")` where `f` is of type
// `FluentBuilder<T1, ..., Tn>`. Calling `table` will return a new
// FluentBuilder of type `FluentBuilder<T1, ...., Tn, Table<int>>`.
// `collections_` (along with all other fields) are moved into the new
// FluentBuilder.
//
// Similar to `collections_`, we also incrementally build `parsers_`: a map
// from a collection's name to a function which can parse messages into it.
template <typename... Cs>
class FluentBuilder {
 public:
  using Parser = std::function<void(const std::vector<std::string>& columns)>;

  // Create a table, scratch, channel, stdin, stdout, or periodic. Note the
  // `&&` at the end of each declaration. This means that these methods can
  // only be invoked on an rvalue-reference, which is necessary since the
  // methods move their contents.

  template <typename... Us>
  FluentBuilder<Cs..., Table<Us...>> table(const std::string& name) && {
    LOG(INFO) << "Adding a table named " << name << ".";
    return AddCollection(std::make_unique<Table<Us...>>(name));
  }

  template <typename... Us>
  FluentBuilder<Cs..., Scratch<Us...>> scratch(const std::string& name) && {
    LOG(INFO) << "Adding a scratch named " << name << ".";
    return AddCollection(std::make_unique<Scratch<Us...>>(name));
  }

  template <typename... Us>
  FluentBuilder<Cs..., Channel<Us...>> channel(const std::string& name) && {
    LOG(INFO) << "Adding a channel named " << name << ".";
    auto c =
        std::make_unique<Channel<Us...>>(name, &network_state_->socket_cache);
    CHECK(parsers_.find(c->Name()) == parsers_.end())
        << "The channel name '" << c->Name()
        << "' is used multiple times. Channel names must be unique.";
    parsers_.insert(std::make_pair(c->Name(), c->GetParser()));
    return AddCollection(std::move(c));
  }

  FluentBuilder<Cs..., Stdin> stdin() && {
    LOG(INFO) << "Adding stdin.";
    auto stdin = std::make_unique<Stdin>();
    stdin_ = stdin.get();
    return AddCollection(std::move(stdin));
  }

  FluentBuilder<Cs..., Stdout> stdout() && {
    LOG(INFO) << "Adding stdout.";
    return AddCollection(std::make_unique<Stdout>());
  }

  FluentBuilder<Cs..., Periodic> periodic(const std::string& name,
                                          const Periodic::period& period) && {
    LOG(INFO) << "Adding Periodic named " << name << ".";
    auto p = std::make_unique<Periodic>(name, period);
    periodics_.push_back(p.get());
    return AddCollection(std::move(p));
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
  FluentExecutor<TypeList<Cs...>, typename std::result_of<F(Cs&...)>::type>
  RegisterRules(const F& f) && {
    return RegisterRulesImpl(f, std::make_index_sequence<sizeof...(Cs)>());
  }

 private:
  // Constructs an empty FluentBuilder. Note that this constructor should
  // only be called when Cs is empty (i.e. sizeof...(Cs) == 0). This private
  // constructor is used primarily by the `fluent` function down below.
  FluentBuilder(const std::string& address, zmq::context_t* const context)
      : network_state_(std::make_unique<NetworkState>(address, context)),
        stdin_(nullptr) {
    static_assert(sizeof...(Cs) == 0,
                  "The FluentBuilder(const std::string& address, "
                  "zmq::context_t* const context) constructor should only be "
                  "called when Cs is empty.");
  }

  // Moves the guts of one FluentBuilder into another.
  FluentBuilder(std::tuple<std::unique_ptr<Cs>...> collections,
                std::map<std::string, Parser> parsers,
                std::unique_ptr<NetworkState> network_state, Stdin* stdin,
                std::vector<Periodic*> periodics)
      : collections_(std::move(collections)),
        parsers_(std::move(parsers)),
        network_state_(std::move(network_state)),
        stdin_(stdin),
        periodics_(std::move(periodics)) {}

  FluentBuilder(FluentBuilder&&) = default;
  FluentBuilder& operator=(FluentBuilder&&) = default;
  DISALLOW_COPY_AND_ASSIGN(FluentBuilder);

  // Return a new FluentBuilder with `c` appended to `collections`.
  template <typename C>
  FluentBuilder<Cs..., C> AddCollection(std::unique_ptr<C> c) {
    std::tuple<std::unique_ptr<Cs>..., std::unique_ptr<C>> collections =
        std::tuple_cat(std::move(collections_), std::make_tuple(std::move(c)));
    return {std::move(collections), std::move(parsers_),
            std::move(network_state_), stdin_, std::move(periodics_)};
  }

  // See `RegisterRules`.
  template <typename F, std::size_t... Is>
  FluentExecutor<TypeList<Cs...>, typename std::result_of<F(Cs&...)>::type>
  RegisterRulesImpl(const F& f, std::index_sequence<Is...>) {
    auto relalgs = f(*std::get<Is>(collections_)...);
    return {std::move(collections_),   std::move(parsers_),
            std::move(network_state_), stdin_,
            std::move(periodics_),     std::move(relalgs)};
  }

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
  // TODO(mwhittaker): Right now, I made things unique_ptr because I was
  // paranoid that as data was being moved from one FluentBuilder to
  // another, pointers to fields would be invalidated. Maybe we don't need the
  // unique_ptr, but I'd need to think harder about it.
  std::tuple<std::unique_ptr<Cs>...> collections_;

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

  // All FluentBuilders are friends of one another.
  template <typename...>
  friend class FluentBuilder;

  // The `fluent` function is used to construct a FluentBuilder without any
  // collections. Recall that looks something like this:
  //
  //   auto f = fluent("tcp://*:8000");
  //     .table<int, char, float>("t")
  //     .scratch<int, int, float>("s")
  //     // and so on...
  friend FluentBuilder<> fluent(const std::string& address,
                                zmq::context_t* const context);
};

// Create an empty FluentBuilder listening on ZeroMQ address `address` using
// the ZeroMQ context `context`.
inline FluentBuilder<> fluent(const std::string& address,
                              zmq::context_t* const context) {
  return FluentBuilder<>(address, context);
}

}  // namespace fluent

#endif  // FLUENT_FLUENT_BUILDER_H_
