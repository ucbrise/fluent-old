#ifndef FLUENT_FLUENT_BUILDER_H_
#define FLUENT_FLUENT_BUILDER_H_

#include <cstddef>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "zmq.hpp"

#include "fluent/channel.h"
#include "fluent/fluent_executor.h"
#include "fluent/network_state.h"
#include "fluent/scratch.h"
#include "fluent/socket_cache.h"
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
template <typename... Ts>
class FluentBuilder {
 public:
  // Create a table, scratch, and channel. Note the `&&` at the end of the
  // declaration. This means that these methods can only be invoked on an
  // rvalue-reference, which is necessary since the methods move their contents.
  template <typename U, typename... Us>
  FluentBuilder<Ts..., Table<U, Us...>> table(const std::string& name) && {
    LOG(INFO) << "Adding a table named " << name << ".";
    return AddCollection(std::make_unique<Table<U, Us...>>(name));
  }

  template <typename U, typename... Us>
  FluentBuilder<Ts..., Scratch<U, Us...>> scratch(const std::string& name) && {
    LOG(INFO) << "Adding a scratch named " << name << ".";
    return AddCollection(std::make_unique<Scratch<U, Us...>>(name));
  }

  template <typename U, typename... Us>
  FluentBuilder<Ts..., Channel<U, Us...>> channel(const std::string& name) && {
    LOG(INFO) << "Adding a channel named " << name << ".";
    return AddCollection(std::make_unique<Channel<U, Us...>>(
        name, &network_state_->socket_cache));
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
  FluentExecutor<std::tuple<std::unique_ptr<Ts>...>,
                 typename std::result_of<F(Ts&...)>::type>
  RegisterRules(const F& f) && {
    return RegisterRulesImpl(f, std::make_index_sequence<sizeof...(Ts)>());
  }

 private:
  using Parser = std::function<void(const std::vector<std::string>& columns)>;

  // Constructs an empty FluentBuilder. Note that this constructor should
  // only be called when Ts is empty (i.e. sizeof...(Ts) == 0).
  FluentBuilder(const std::string& address)
      : network_state_(std::make_unique<NetworkState>(address)) {
    static_assert(sizeof...(Ts) == 0,
                  "The FluentBuilder(const std::string& address) "
                  "constructor should only be called when Ts is empty.");
  }

  // Moves the guts of one FluentBuilder into another.
  FluentBuilder(std::tuple<std::unique_ptr<Ts>...> collections,
                std::map<std::string, Parser> parsers,
                std::unique_ptr<NetworkState> network_state)
      : collections_(std::move(collections)),
        parsers_(std::move(parsers)),
        network_state_(std::move(network_state)) {}

  // Return a new FluentBuilder with `c` and `c->GetParser()` appended to
  // `collections_` and `parsers_`.
  template <typename C>
  FluentBuilder<Ts..., C> AddCollection(std::unique_ptr<C> c) {
    CHECK(parsers_.find(c->Name()) == parsers_.end())
        << "The collection name '" << c->Name()
        << "' is used multiple times. Collection names must be unique.";

    parsers_.insert(std::make_pair(c->Name(), c->GetParser()));
    std::tuple<std::unique_ptr<Ts>..., std::unique_ptr<C>> collections =
        std::tuple_cat(std::move(collections_), std::make_tuple(std::move(c)));
    return {std::move(collections), std::move(parsers_),
            std::move(network_state_)};
  }

  template <typename F, std::size_t... Is>
  FluentExecutor<std::tuple<std::unique_ptr<Ts>...>,
                 typename std::result_of<F(Ts&...)>::type>
  RegisterRulesImpl(const F& f, std::index_sequence<Is...>) {
    auto relalgs = f(*std::get<Is>(collections_)...);
    return {std::move(collections_), std::move(parsers_),
            std::move(network_state_), std::move(relalgs)};
  }

  // TODO(mwhittaker): Right now, I made things unique_ptr because I was
  // paranoid that as data was being moved from one FluentBuilder to
  // another, pointers to fields would be invalidated. Maybe we don't need the
  // unique_ptr, but I'd need to think harder about it.
  std::tuple<std::unique_ptr<Ts>...> collections_;
  std::map<std::string, Parser> parsers_;
  std::unique_ptr<NetworkState> network_state_;

  template <typename...>
  friend class FluentBuilder;

  friend FluentBuilder<> fluent(const std::string& address);
};

// Create an empty FluentBuilder listening on ZeroMQ address `address`.
inline FluentBuilder<> fluent(const std::string& address) {
  return FluentBuilder<>(address);
}

}  // namespace fluent

#endif  // FLUENT_FLUENT_BUILDER_H_
