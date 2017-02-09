#include "fluent/channel.h"

#include <set>
#include <string>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "fluent/socket_cache.h"
#include "zmq_util/zmq_util.h"

namespace fluent {

using ::testing::UnorderedElementsAreArray;

TEST(Channel, SimpleTest) {
  zmq::context_t context(1);
  SocketCache cache(&context);
  Channel<std::string, int, int> c("c", &cache);

  const std::string a_address = "inproc://a";
  const std::string b_address = "inproc://b";
  zmq::socket_t a(context, ZMQ_PULL);
  zmq::socket_t b(context, ZMQ_PULL);
  a.bind(a_address);
  b.bind(b_address);

  using Tuple = std::tuple<std::string, int, int>;
  using TupleSet = std::set<Tuple>;
  Tuple ta{a_address, 1, 1};
  Tuple tb{b_address, 2, 2};

  EXPECT_THAT(c.Get(), UnorderedElementsAreArray(TupleSet{}));
  c.Add(ta);
  EXPECT_THAT(c.Get(), UnorderedElementsAreArray(TupleSet{}));
  c.Add(tb);
  EXPECT_THAT(c.Get(), UnorderedElementsAreArray(TupleSet{}));

  std::vector<zmq::message_t> a_messages = zmq_util::recv_msgs(&a);
  ASSERT_EQ(a_messages.size(), static_cast<std::size_t>(4));
  EXPECT_EQ("c", zmq_util::message_to_string(a_messages[0]));
  EXPECT_EQ(a_address, zmq_util::message_to_string(a_messages[1]));
  EXPECT_EQ("1", zmq_util::message_to_string(a_messages[2]));
  EXPECT_EQ("1", zmq_util::message_to_string(a_messages[3]));

  std::vector<zmq::message_t> b_messages = zmq_util::recv_msgs(&b);
  ASSERT_EQ(b_messages.size(), static_cast<std::size_t>(4));
  EXPECT_EQ("c", zmq_util::message_to_string(b_messages[0]));
  EXPECT_EQ(b_address, zmq_util::message_to_string(b_messages[1]));
  EXPECT_EQ("2", zmq_util::message_to_string(b_messages[2]));
  EXPECT_EQ("2", zmq_util::message_to_string(b_messages[3]));

  // TODO(mwhittaker): Test Tick. It's a bit annoying to do because I have to
  // first force some tuples into the Channel.
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
