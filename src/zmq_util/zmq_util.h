#ifndef ZMQ_UTIL_ZMQ_UTIL_H_
#define ZMQ_UTIL_ZMQ_UTIL_H_

#include <cstring>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "zmq.hpp"

namespace fluent {

// Converts the data within a `zmq::message_t` into a string.
std::string message_to_string(const zmq::message_t& message);

// Converts a string into a `zmq::message_t`.
zmq::message_t string_to_message(const std::string& s);

// `send` a string over the socket.
void send_string(const std::string& s, zmq::socket_t* socket);

// `recv` a string over the socket.
std::string recv_string(zmq::socket_t* socket);

// `send` a multipart message.
void send_msgs(std::vector<zmq::message_t> msgs, zmq::socket_t* socket);

// `recv` a multipart message.
std::vector<zmq::message_t> recv_msgs(zmq::socket_t* socket);

}  // namespace fluent

#endif  // ZMQ_UTIL_ZMQ_UTIL_H_
