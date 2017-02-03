#include "fluent/socket_cache.h"

#include "gtest/gtest.h"
#include "zmq.hpp"

#include "zmq_util/zmq_util.h"

namespace fluent {

TEST(SocketCache, ThreeSockets) {
  const std::string a_address = "inproc://a";
  const std::string b_address = "inproc://b";
  const std::string c_address = "inproc://c";

  zmq::context_t context(1);
  zmq::socket_t a(context, ZMQ_REP);
  zmq::socket_t b(context, ZMQ_REP);
  zmq::socket_t c(context, ZMQ_REP);
  a.bind(a_address);
  b.bind(b_address);
  c.bind(c_address);

  SocketCache cache(&context);
  for (int i = 0; i < 2; ++i) {
    send_string("foo", &cache[a_address]);
    send_string("bar", &cache[b_address]);
    send_string("baz", &cache[c_address]);
    EXPECT_EQ("foo", recv_string(&a));
    EXPECT_EQ("bar", recv_string(&b));
    EXPECT_EQ("baz", recv_string(&c));

    send_string("foo", &a);
    send_string("bar", &b);
    send_string("baz", &c);
    EXPECT_EQ("foo", recv_string(&cache[a_address]));
    EXPECT_EQ("bar", recv_string(&cache[b_address]));
    EXPECT_EQ("baz", recv_string(&cache[c_address]));
  }
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
