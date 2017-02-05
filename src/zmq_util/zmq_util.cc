#include "zmq_util/zmq_util.h"

#include <iomanip>
#include <ios>

#include "glog/logging.h"

namespace fluent {
namespace zmq_util {

std::string message_to_string(const zmq::message_t& message) {
  return std::string(static_cast<const char*>(message.data()), message.size());
}

zmq::message_t string_to_message(const std::string& s) {
  zmq::message_t msg(s.size());
  memcpy(msg.data(), s.c_str(), s.size());
  return msg;
}

void send_string(const std::string& s, zmq::socket_t* socket) {
  CHECK_NOTNULL(socket);
  socket->send(string_to_message(s));
}

std::string recv_string(zmq::socket_t* socket) {
  CHECK_NOTNULL(socket);
  zmq::message_t message;
  socket->recv(&message);
  return message_to_string(message);
}

void send_msgs(std::vector<zmq::message_t> msgs, zmq::socket_t* socket) {
  CHECK_NOTNULL(socket);
  for (std::size_t i = 0; i < msgs.size(); ++i) {
    socket->send(msgs[i], i == msgs.size() - 1 ? 0 : ZMQ_SNDMORE);
  }
}

std::vector<zmq::message_t> recv_msgs(zmq::socket_t* socket) {
  CHECK_NOTNULL(socket);
  std::vector<zmq::message_t> msgs;
  int more = true;
  std::size_t more_size = sizeof(more);
  while (more) {
    msgs.emplace_back();
    socket->recv(&msgs.back());
    socket->getsockopt(ZMQ_RCVMORE, static_cast<void*>(&more), &more_size);
  }
  return msgs;
}

}  // namespace zmq_util
}  // namespace fluent
