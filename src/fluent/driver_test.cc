#include "fluent/driver.h"

#include <cstddef>

#include "gtest/gtest.h"

namespace fluent {

TEST(Driver, SimpleTick) {
  const std::string address = "tcp://*:8000";
  // clang-format off
  auto d = driver(address)
    .table<int, char, float>("t")
    .scratch<int, int, float>("s")
    .channel<std::string, float, char>("c");
  // clang-format on

  d.Tick([](Table<int, char, float>* t, Scratch<int, int, float>* s,
            Channel<std::string, float, char>* c) {
    EXPECT_EQ("t", t->Name());
    EXPECT_EQ("s", s->Name());
    EXPECT_EQ("c", c->Name());
  });
}

TEST(Driver, ClearScratches) {
  auto d = driver("inproc://foo").scratch<int, int>("s");

  d.Tick([&](Scratch<int, int>* s) {
    EXPECT_EQ(s->Get().size(), static_cast<std::size_t>(0));
    s->Add(std::make_tuple(42, 43));
    EXPECT_EQ(s->Get().size(), static_cast<std::size_t>(1));
  });

  d.Tick([&](Scratch<int, int>* s) {
    EXPECT_EQ(s->Get().size(), static_cast<std::size_t>(0));
  });
}

TEST(Driver, SimpleCommunication) {
  const std::string ping_port = "8000";
  const std::string pong_port = "8001";
  const std::string ping_address = "tcp://localhost:" + ping_port;
  const std::string pong_address = "tcp://localhost:" + pong_port;
  auto ping = driver("tcp://*:" + ping_port).channel<std::string, int>("c");
  auto pong = driver("tcp://*:" + pong_port).channel<std::string, int>("c");

  ping.Tick([&](Channel<std::string, int>* c) {
    c->Add(std::make_tuple(pong_address, 42));
  });

  pong.Receive();
  pong.Tick([&](Channel<std::string, int>* c) {
    ASSERT_EQ(c->Get().size(), static_cast<std::size_t>(1));
    auto iter = c->Get().begin();
    EXPECT_EQ(std::get<0>(*iter), pong_address);
    EXPECT_EQ(std::get<1>(*iter), 42);
    c->Add(std::make_tuple(ping_address, 14));
  });

  ping.Receive();
  ping.Tick([&](Channel<std::string, int>* c) {
    ASSERT_EQ(c->Get().size(), static_cast<std::size_t>(1));
    auto iter = c->Get().begin();
    EXPECT_EQ(std::get<0>(*iter), ping_address);
    EXPECT_EQ(std::get<1>(*iter), 14);
  });
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
