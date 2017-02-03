#ifndef FLUENT_SOCKET_CACHE_H_
#define FLUENT_SOCKET_CACHE_H_

#include <map>
#include <string>

#include "zmq.hpp"

namespace fluent {

// A SocketCache is a map from ZeroMQ addresses to ZeroMQ sockets. The socket
// corresponding to address `address` can be retrieved from a SocketCache
// `cache` with `cache[address]` or `cache.At(address)`. If a socket with a
// given address is not in the cache when it is requested, one is created and
// connected to the address. An example:
//
//   zmq::context_t context(1);
//   SocketCache cache(&context);
//   // This will create a socket and connect it to "inproc://a".
//   zmq::socket_t& a = cache["inproc://a"];
//   // This will not createa new socket. It will return the socket created in
//   // the previous line. In other words, a and the_same_a_as_before are
//   // references to the same socket.
//   zmq::socket_t& the_same_a_as_before = cache["inproc://a"];
//   // cache.At("inproc://a") is 100% equivalent to cache["inproc://a"].
//   zmq::socket_t& another_a = cache.At("inproc://a");
//
// TODO(mwhittaker): Figure out what type of socket we should use and add that
// to the documentation. For now, we use REQ/REP sockets because they are the
// simplest, but we'll need a more flexible form of socket. Perhaps,
// SocketCache can take the socket type as an argument.
class SocketCache {
 public:
  explicit SocketCache(zmq::context_t* context) : context_(context) {}
  zmq::socket_t& At(const std::string& addr);
  zmq::socket_t& operator[](const std::string& addr);

 private:
  zmq::context_t* context_;
  std::map<std::string, zmq::socket_t> cache_;
};

}  // namespace fluent

#endif  // FLUENT_SOCKET_CACHE_H_
