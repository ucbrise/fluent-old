#include "collections/channel.h"

#include <cstddef>

#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "common/mock_pickler.h"
#include "zmq_util/socket_cache.h"
#include "zmq_util/zmq_util.h"

namespace fluent {
namespace collections {

using common::MockPickler;

TEST(Channel, ChannelStartsEmpty) {
  zmq::context_t context(1);
  zmq_util::SocketCache cache(&context);
  Channel<MockPickler, std::string, int> c(42, "c", {{"addr", "x"}}, &cache);
  std::map<std::tuple<std::string, int>, CollectionTupleIds> expected;
  EXPECT_EQ(c.Get(), expected);
}

TEST(Channel, Merge) {
  zmq::context_t context(1);
  zmq_util::SocketCache cache(&context);
  Channel<MockPickler, std::string, int> c(42, "c", {{"addr", "x"}}, &cache);
  std::map<std::tuple<std::string, int>, CollectionTupleIds> expected;

  const std::string a_address = "inproc://a";
  const std::string b_address = "inproc://b";
  zmq::socket_t a(context, ZMQ_PULL);
  zmq::socket_t b(context, ZMQ_PULL);
  a.bind(a_address);
  b.bind(b_address);

  c.Merge({a_address, 1}, 1, 0);
  c.Merge({b_address, 2}, 2, 0);
  c.Merge({a_address, 3}, 3, 0);
  c.Merge({b_address, 4}, 4, 0);
  c.Merge({a_address, 5}, 5, 0);
  c.Merge({b_address, 6}, 6, 0);
  expected = {};
  EXPECT_EQ(c.Get(), expected);

  for (int i = 1; i <= 6; ++i) {
    zmq::socket_t* recipient = i % 2 == 0 ? &b : &a;
    const std::string& address = i % 2 == 0 ? b_address : a_address;
    std::vector<zmq::message_t> messages = zmq_util::recv_msgs(recipient);

    ASSERT_EQ(messages.size(), static_cast<std::size_t>(5));
    EXPECT_EQ("42", zmq_util::message_to_string(messages[0]));
    EXPECT_EQ("c", zmq_util::message_to_string(messages[1]));
    EXPECT_EQ("0", zmq_util::message_to_string(messages[2]));
    EXPECT_EQ(address, zmq_util::message_to_string(messages[3]));
    EXPECT_EQ(std::to_string(i), zmq_util::message_to_string(messages[4]));
  }
}

TEST(Channel, Receive) {
  zmq::context_t context(1);
  zmq_util::SocketCache cache(&context);
  Channel<MockPickler, std::string, int> c(42, "c", {{"addr", "x"}}, &cache);
  std::map<std::tuple<std::string, int>, CollectionTupleIds> expected;

  c.Receive({"a", 1}, 0xA, 0);
  expected = {{{"a", 1}, {0xA, {0}}}};
  EXPECT_EQ(c.Get(), expected);

  c.Receive({"b", 2}, 0xB, 1);
  expected = {{{"a", 1}, {0xA, {0}}}, {{"b", 2}, {0xB, {1}}}};
  EXPECT_EQ(c.Get(), expected);

  c.Receive({"b", 2}, 0xB, 2);
  expected = {{{"a", 1}, {0xA, {0}}}, {{"b", 2}, {0xB, {1, 2}}}};
  EXPECT_EQ(c.Get(), expected);
}

TEST(Channel, TickClearsChannel) {
  zmq::context_t context(1);
  zmq_util::SocketCache cache(&context);
  Channel<MockPickler, std::string, int> c(42, "c", {{"addr", "x"}}, &cache);
  std::map<std::tuple<std::string, int>, CollectionTupleIds> expected;

  c.Receive({"a", 1}, 0xA, 0);
  expected = {{{"a", 1}, {0xA, {0}}}};
  EXPECT_EQ(c.Get(), expected);

  c.Tick();
  expected = {};
  EXPECT_EQ(c.Get(), expected);
}

}  // namespace collections
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
