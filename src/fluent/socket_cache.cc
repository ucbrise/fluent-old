#include "fluent/socket_cache.h"

#include <utility>

namespace fluent {

zmq::socket_t& SocketCache::At(const std::string& addr) {
  auto iter = cache_.find(addr);
  if (iter != cache_.end()) {
    return iter->second;
  }
  // See the TODO in the header file discussing which socket type to use.
  zmq::socket_t socket(*context_, ZMQ_REQ);
  socket.connect(addr);
  auto p = cache_.insert(std::make_pair(addr, std::move(socket)));
  return p.first->second;
}

zmq::socket_t& SocketCache::operator[](const std::string& addr) {
  return At(addr);
}

}  // namespace fluent
