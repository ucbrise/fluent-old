#include "fluent/fluent_executor.h"

#include <cstddef>

#include <tuple>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "fluent/channel.h"
#include "fluent/fluent_builder.h"
#include "ra/all.h"

using ::testing::UnorderedElementsAreArray;

namespace fluent {

TEST(FluentExecutor, SimpleProgram) {
  zmq::context_t context(1);

  // clang-format off
  auto f = fluent("inproc://yolo", &context)
    .table<int>("t")
    .scratch<int, int, float>("s")
    .channel<std::string, float, char>("c")
    .RegisterRules([](auto& t, auto& s, auto& c) {
      return std::make_tuple(
        t <= (t.Iterable() | ra::count()),
        t <= (s.Iterable() | ra::count()),
        t <= (c.Iterable() | ra::count())
      );
    });
  // clang-format on

  using T = std::set<std::tuple<int>>;
  using S = std::set<std::tuple<int, int, float>>;
  using C = std::set<std::tuple<std::string, float, char>>;

  f.Tick();
  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(T{{0}}));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(S{}));
  EXPECT_THAT(f.Get<2>().Get(), UnorderedElementsAreArray(C{}));

  f.Tick();
  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(T{{0}, {1}}));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(S{}));
  EXPECT_THAT(f.Get<2>().Get(), UnorderedElementsAreArray(C{}));

  f.Tick();
  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(T{{0}, {1}, {2}}));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(S{}));
  EXPECT_THAT(f.Get<2>().Get(), UnorderedElementsAreArray(C{}));
}

TEST(FluentExecutor, ClearScrathes) {
  zmq::context_t context(1);

  // clang-format off
  auto f = fluent("inproc://yolo", &context)
    .table<int>("t")
    .scratch<int>("s")
    .RegisterRules([](auto& t, auto& s) {
      return std::make_tuple(t <= (s.Iterable() | ra::count()));
    });
  // clang-format on

  using T = std::set<std::tuple<int>>;
  using S = std::set<std::tuple<int>>;
  for (int i = 0; i < 3; ++i) {
    f.Tick();
    EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(T{{0}}));
    EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(S{}));
  }
}

TEST(FluentExecutor, SimpleCommunication) {
  auto reroute = [](const std::string& s) {
    return [s](const std::tuple<std::string, int>& t) {
      return std::make_tuple(s, std::get<1>(t));
    };
  };

  zmq::context_t context(1);
  // clang-format off
  auto ping = fluent("inproc://ping", &context)
    .channel<std::string, int>("c")
    .RegisterRules([&reroute](auto& c) {
      return std::make_tuple(
        c <= (c.Iterable() | ra::map(reroute("inproc://pong")))
      );
    });
  auto pong = fluent("inproc://pong", &context)
    .channel<std::string, int>("c")
    .RegisterRules([&reroute](auto& c) {
      return std::make_tuple(
        c <= (c.Iterable() | ra::map(reroute("inproc://ping")))
      );
    });
  // clang-format on

  using C = std::set<std::tuple<std::string, int>>;
  ping.MutableGet<0>().Add(std::make_tuple("inproc://pong", 42));

  for (int i = 0; i < 3; ++i) {
    pong.Receive();
    EXPECT_THAT(pong.Get<0>().Get(),
                UnorderedElementsAreArray(C{{"inproc://pong", 42}}));
    pong.Tick();
    EXPECT_THAT(pong.Get<0>().Get(), UnorderedElementsAreArray(C{}));

    ping.Receive();
    EXPECT_THAT(ping.Get<0>().Get(),
                UnorderedElementsAreArray(C{{"inproc://ping", 42}}));
    ping.Tick();
    EXPECT_THAT(ping.Get<0>().Get(), UnorderedElementsAreArray(C{}));
  }
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
