#include "fluent/channel.h"

#include <set>
#include <string>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "fluent/mock_pickler.h"
#include "fluent/socket_cache.h"
#include "ra/all.h"
#include "zmq_util/zmq_util.h"

namespace fluent {

using ::testing::UnorderedElementsAreArray;

TEST(Channel, SimpleMerge) {
  zmq::context_t context(1);
  SocketCache cache(&context);
  Channel<MockPickler, std::string, int, int> c(42, "c", {{"addr", "x", "y"}},
                                                &cache);

  const std::string a_address = "inproc://a";
  const std::string b_address = "inproc://b";
  zmq::socket_t a(context, ZMQ_PULL);
  zmq::socket_t b(context, ZMQ_PULL);
  a.bind(a_address);
  b.bind(b_address);

  std::set<std::tuple<std::string, int, int>> empty = {};
  std::set<std::tuple<std::string, int, int>> msgs = {
      {a_address, 1, 1}, {b_address, 2, 2}, {a_address, 3, 3},
      {b_address, 4, 4}, {a_address, 5, 5}, {b_address, 6, 6},
      {a_address, 7, 7}, {b_address, 8, 8}};

  EXPECT_THAT(c.Get(), UnorderedElementsAreArray(empty));
  c.Merge(msgs, 9001);
  EXPECT_THAT(c.Get(), UnorderedElementsAreArray(empty));

  for (int i = 1; i < 9; ++i) {
    std::vector<zmq::message_t> messages =
        i % 2 == 0 ? zmq_util::recv_msgs(&b) : zmq_util::recv_msgs(&a);
    ASSERT_EQ(messages.size(), static_cast<std::size_t>(6));
    EXPECT_EQ("42", zmq_util::message_to_string(messages[0]));
    EXPECT_EQ("c", zmq_util::message_to_string(messages[1]));
    EXPECT_EQ("9001", zmq_util::message_to_string(messages[2]));
    EXPECT_EQ(i % 2 == 0 ? b_address : a_address,
              zmq_util::message_to_string(messages[3]));
    EXPECT_EQ(std::to_string(i), zmq_util::message_to_string(messages[4]));
    EXPECT_EQ(std::to_string(i), zmq_util::message_to_string(messages[5]));
  }
}

TEST(Channel, TickClearsChannel) {
  using Tuple = std::tuple<std::string, int, int>;
  using TupleSet = std::set<Tuple>;

  zmq::context_t context(1);
  SocketCache cache(&context);
  Channel<MockPickler, std::string, int, int> c(42, "c", {{"addr", "x", "y"}},
                                                &cache);

  c.ts_.insert(Tuple("foo", 1, 1));
  c.ts_.insert(Tuple("bar", 2, 2));
  c.ts_.insert(Tuple("baz", 3, 3));

  EXPECT_THAT(c.Get(), UnorderedElementsAreArray(TupleSet{
                           {"foo", 1, 1}, {"bar", 2, 2}, {"baz", 3, 3}}));
  c.Tick();
  EXPECT_THAT(c.Get(), UnorderedElementsAreArray(TupleSet{}));
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
