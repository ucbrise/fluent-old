#ifndef FLUENT_DRIVER_H_
#define FLUENT_DRIVER_H_

#include <cassert>
#include <cstddef>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "zmq.hpp"

#include "fluent/channel.h"
#include "fluent/scratch.h"
#include "fluent/socket_cache.h"
#include "fluent/table.h"

namespace fluent {

// A driver drives the exeution of Fluent program. It is best explained through
// an example.
//
//   // Each driver contains a socket which listens for messages from other
//   // Fluent nodes. Here, our driver will listen on "tcp://*:8000".
//   const std::string address = "tcp://*:8000";
//
//   // This driver has four collections:
//   //   - a 3-column table named "t1" with types [int, char, float]
//   //   - a 2-column table named "t2" with types [float, int]
//   //   - a 3-column scratch named "s" with types [int, int, float]
//   //   - a 3-column channel named "c" with types [std::string, float, char]
//   auto d = driver(address)
//     .table<int, char, float>("t1")
//     .table<float, int>("t2")
//     .scratch<int, int, float>("s")
//     .channel<std::string, float, char>("c");
//
//   // Execute a single tick of execution. Initially, all the collections will
//   // be empty. Here, we simply print their names.
//   d.Tick([](
//     Table<int, char, float>* t1,
//     Table<float, int>* t2,
//     Scratch<int, int, float>* s,
//     Channel<std::string, float, char>* c
//   ) {
//     std::cout << t1->Name() << std::endl; // "t1"
//     std::cout << t2->Name() << std::endl; // "t2"
//     std::cout << s->Name() << std::endl;  // "s"
//     std::cout << c->Name() << std::endl;  // "c"
//   });
//
//   // Run a Fluent program. The driver will repeatedly receive messages from
//   // other Fluent nodes, execute a tick, and clear channels and scratches.
//   d.Tick([](
//     Table<int, char, float>* t1,
//     Table<float, int>* t2,
//     Scratch<int, int, float>* s,
//     Channel<std::string, float, char>* c
//   ) {
//     // Do some super crazy monotonic stuff!
//   });
//
// In order to support the nice `.table`, `.scratch`, and `.channel` syntax,
// Driver requires quite a bit of metaprogramming.
//
// A Driver<T1, ..., Tn> drives a Fluent program with n collections. Each Ti is
// either a Table<Us...>, Scratch<Us...>, and Channel<Us...>. Pointers
// to the collections are stored `collections_` which is of type
// `std::tuple<std::unique_ptr<T1>, ..., std::unique_ptr<Tn>>`.
//
// Imagine we call `d.table<int>("foo")` where `d` is of type `Driver<T1, ...,
// Tn>`. Calling `table` will return a new driver of type `Driver<T1, ...., Tn,
// Table<int>>`. `collections_` (along with all other fields) are moved into
// the new driver.
//
// Similar to `collections_`, we also incrementally build `parsers_`: a map
// from a collection's name to a function which can parse messages into it.
template <typename... Ts>
class Driver {
 public:
  // Create a table, scratch, and channel. Note the `&&` at the end of the
  // declaration. This means that these methods can only be invoked on an
  // rvalue-reference, which is necessary since the methods move their contents.
  template <typename U, typename... Us>
  Driver<Ts..., Table<U, Us...>> table(const std::string& name) && {
    return AddCollection(std::make_unique<Table<U, Us...>>(name));
  }

  template <typename U, typename... Us>
  Driver<Ts..., Scratch<U, Us...>> scratch(const std::string& name) && {
    return AddCollection(std::make_unique<Scratch<U, Us...>>(name));
  }

  template <typename U, typename... Us>
  Driver<Ts..., Channel<U, Us...>> channel(const std::string& name) && {
    return AddCollection(
        std::make_unique<Channel<U, Us...>>(name, socket_cache_.get()));
  }

  // Run `f` on the current state of the collections. `f` takes a pointer to
  // each collection in the order the collections were added to the driver.
  void Tick(std::function<void(Ts*...)> f) {
    TickImpl(f, std::make_index_sequence<sizeof...(Ts)>());
  }

  // (Potentially) block and receive messages sent by other Fluent nodes.
  // Receiving a message will insert it into the appropriate channel.
  void Receive() {
    std::vector<zmq::message_t> msgs = zmq_util::recv_msgs(socket_.get());
    std::vector<std::string> strings;
    for (std::size_t i = 1; i < msgs.size(); ++i) {
      strings.push_back(zmq_util::message_to_string(msgs[i]));
    }
    // TODO(mwhittaker): Check to see if zmq_util::message_to_string(msgs[0])
    // is in parsers_, logging a warning or something if it is not.
    parsers_[zmq_util::message_to_string(msgs[0])](strings);
  }

  // Runs a fluent program.
  //
  // TODO(mwhittaker): Should it be Receive() then Tick() or Tick then
  // Receive()?
  void Run(std::function<void(Ts*...)> f) {
    while (true) {
      Tick(f);
      Receive();
    }
  }

 private:
  using Collections = std::tuple<std::unique_ptr<Ts>...>;
  using Parser = std::function<void(const std::vector<std::string>& columns)>;
  using Parsers = std::map<std::string, Parser>;

  // Constructs an empty driver. This constructor should only be called when Ts
  // is empty.
  Driver(const std::string& address)
      : context_(std::make_unique<zmq::context_t>(1)),
        socket_(std::make_unique<zmq::socket_t>(*context_, ZMQ_REP)),
        socket_cache_(std::make_unique<SocketCache>(context_.get())) {
    static_assert(sizeof...(Ts) == 0, "");
    socket_->bind(address);
  }

  // Moves the guts of one Driver into another.
  Driver(Collections collections, Parsers parsers,
         std::unique_ptr<zmq::context_t> context,
         std::unique_ptr<zmq::socket_t> socket,
         std::unique_ptr<SocketCache> socket_cache)
      : collections_(std::move(collections)),
        parsers_(std::move(parsers)),
        context_(std::move(context)),
        socket_(std::move(socket)),
        socket_cache_(std::move(socket_cache)) {}

  // Return a new Driver with `c` and `c->GetParser()` appended to
  // `collections_` and `parsers_`.
  template <typename C>
  Driver<Ts..., C> AddCollection(std::unique_ptr<C> c) {
    // TODO(mwhittaker): Right now, we check that the name is not already taken
    // with an assert. We should change return types around to use something
    // like Google's `StatusOr` instead.
    assert(parsers_.find(c->Name()) == parsers_.end());

    parsers_.insert(std::make_pair(c->Name(), c->GetParser()));
    std::tuple<std::unique_ptr<Ts>..., std::unique_ptr<C>> collections =
        std::tuple_cat(std::move(collections_), std::make_tuple(std::move(c)));
    return {std::move(collections), std::move(parsers_), std::move(context_),
            std::move(socket_), std::move(socket_cache_)};
  }

  // `TickCollections<0>` calls `Tick` on every collection in `collection_`.
  //
  // Since `collection_` is a tuple, we have to do a bit of metaprogramming.
  //   - Invoking `TickCollections<I>` where `I > sizeof...(Ts)` will not
  //     compile.
  //   - Invoking `TickCollections<I>` where `I == sizeof...(Ts)` will invoke
  //     the first definition.
  //   - Invoking `TickCollections<I>` where `I < sizeof...(Ts)` will invoke the
  //     second definition.
  // `TickCollections<0>` invokes `TickCollections<1>` which invokes
  // `TickCollections<2>` and so on until we hit
  // `TickCollections<sizeof...(Ts)>` and return. Each `TickCollections<I>`
  // call calls `Tick` on the `I`th collection in `collections_`.
  template <std::size_t I>
  typename std::enable_if<I == sizeof...(Ts)>::type TickCollections() {}

  template <std::size_t I>
  typename std::enable_if<I != sizeof...(Ts)>::type TickCollections() {
    if (I != sizeof...(Ts)) {
      std::get<I>(collections_)->Tick();
      TickCollections<I + 1>();
    }
  }

  // `TickImpl` calls `f` on the current state of the collections and then
  // clears the scratches and channels.
  template <std::size_t... Is>
  void TickImpl(std::function<void(Ts*...)> f, std::index_sequence<Is...>) {
    f(std::get<Is>(collections_).get()...);
    TickCollections<0>();
  }

  Collections collections_;
  Parsers parsers_;

  // TODO(mwhittaker): Right now, I made things unique_ptrs because I was
  // paranoid that as data was being moved from one Driver to another, pointers
  // to fields would be invalidated. Maybe we don't need the unique_ptrs, but
  // I'd need to think harder about it.
  std::unique_ptr<zmq::context_t> context_;
  std::unique_ptr<zmq::socket_t> socket_;
  std::unique_ptr<SocketCache> socket_cache_;

  friend Driver<> driver(const std::string& address);

  template <typename...>
  friend class Driver;
};

// Create an empty driver listening on ZeroMQ address `address`.
inline Driver<> driver(const std::string& address) { return Driver<>(address); }

}  // namespace fluent

#endif  // FLUENT_DRIVER_H_
