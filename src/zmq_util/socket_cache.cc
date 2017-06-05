#include "zmq_util/socket_cache.h"

#include <utility>

namespace fluent {
namespace zmq_util {

zmq::socket_t& SocketCache::At(const std::string& addr) {
  auto iter = cache_.find(addr);
  if (iter != cache_.end()) {
    return iter->second;
  }
  zmq::socket_t socket(*context_, ZMQ_PUSH);
  socket.connect(addr);
  auto p = cache_.insert(std::make_pair(addr, std::move(socket)));
  return p.first->second;
}

zmq::socket_t& SocketCache::operator[](const std::string& addr) {
  return At(addr);
}

}  // namespace zmq_util
}  // namespace fluent
