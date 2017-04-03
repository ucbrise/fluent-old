#include "fluent/fluent_executor.h"

#include <cstddef>

#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "fluent/channel.h"
#include "fluent/fluent_builder.h"
#include "fluent/infix.h"
#include "postgres/connection_config.h"
#include "postgres/noop_client.h"
#include "ra/all.h"
#include "testing/captured_stdout.h"

namespace pg = fluent::postgres;

using ::testing::UnorderedElementsAreArray;

namespace fluent {
namespace {

auto noopfluent(const std::string& name, const std::string& address,
                zmq::context_t* context,
                const pg::ConnectionConfig& connection_config) {
  return fluent<pg::NoopClient, Hash, pg::ToSql>(name, address, context,
                                                 connection_config);
}

}  // namespace

TEST(FluentExecutor, SimpleProgram) {
  zmq::context_t context(1);
  postgres::ConnectionConfig connection_config;
  auto f = noopfluent("name", "inproc://yolo", &context, connection_config)
               .table<std::size_t>("t")
               .scratch<int, int, float>("s")
               .channel<std::string, float, char>("c")
               .RegisterRules([](auto& t, auto& s, auto& c) {
                 using namespace fluent::infix;
                 return std::make_tuple(t <= (t.Iterable() | ra::count()),
                                        t <= (s.Iterable() | ra::count()),
                                        t <= (c.Iterable() | ra::count()));
               });

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

TEST(FluentExecutor, SimpleBootstrap) {
  using Tuples = std::set<std::tuple<int>>;
  Tuples xs = {{1}, {2}, {3}, {4}, {5}};

  zmq::context_t context(1);
  postgres::ConnectionConfig connection_config;
  auto f =
      noopfluent("name", "inproc://yolo", &context, connection_config)
          .table<int>("t")
          .scratch<int>("s")
          .RegisterBootstrapRules([&xs](auto& t, auto& s) {
            using namespace fluent::infix;
            return std::make_tuple(t <= ra::make_iterable("xs", &xs),
                                   s <= ra::make_iterable("xs", &xs));
          })
          .RegisterRules([&xs](auto&, auto&) { return std::make_tuple(); });

  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(Tuples{}));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(Tuples{}));
  f.BootstrapTick();
  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(xs));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(Tuples{}));
}

TEST(FluentExecutor, MildlyComplexProgram) {
  auto int_tuple_to_string = [](const std::tuple<int>& t) {
    return std::tuple<std::string>(std::to_string(std::get<0>(t)));
  };

  zmq::context_t context(1);
  postgres::ConnectionConfig connection_config;
  auto f =
      noopfluent("name", "inproc://yolo", &context, connection_config)
          .table<std::size_t>("t")
          .scratch<std::size_t>("s")
          .stdout()
          .RegisterRules([&](auto& t, auto& s, auto& stdout) {
            using namespace fluent::infix;
            auto a = t <= (t.Iterable() | ra::count());
            auto b = t += t.Iterable();
            auto c = t -= s.Iterable();
            auto d = s <= (t.Iterable() | ra::count());
            auto e = stdout <= (s.Iterable() | ra::map(int_tuple_to_string));
            auto f = stdout += (s.Iterable() | ra::map(int_tuple_to_string));
            return std::make_tuple(a, b, c, d, e, f);
          });

  using T = std::set<std::tuple<int>>;
  CapturedStdout captured;

  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(T{}));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(T{}));
  EXPECT_STREQ("", captured.Get().c_str());

  f.Tick();
  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(T{{0}}));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(T{}));
  EXPECT_STREQ("1\n1\n", captured.Get().c_str());

  f.Tick();
  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(T{{0}, {1}}));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(T{}));
  EXPECT_STREQ("1\n1\n2\n2\n", captured.Get().c_str());
}

TEST(FluentExecutor, ComplexProgram) {
  using Tuple = std::tuple<int>;
  using T = std::set<Tuple>;
  using S = std::set<Tuple>;

  auto plus_one_times_two = [](const std::tuple<std::size_t>& t) {
    return std::tuple<std::size_t>((1 + std::get<0>(t)) * 2);
  };
  auto is_even = [](const auto& t) { return std::get<0>(t) % 2 == 0; };

  zmq::context_t context(1);
  postgres::ConnectionConfig connection_config;
  auto f = noopfluent("name", "inproc://yolo", &context, connection_config)
               .table<std::size_t>("t")
               .scratch<std::size_t>("s")
               .RegisterRules([plus_one_times_two, is_even](auto& t, auto& s) {
                 using namespace fluent::infix;
                 auto a = t += (s.Iterable() | ra::count());
                 auto b = t <= (t.Iterable() | ra::map(plus_one_times_two));
                 auto c = s <= t.Iterable();
                 auto d = t -= (s.Iterable() | ra::filter(is_even));
                 return std::make_tuple(a, b, c, d);
               });

  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(T{}));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(S{}));

  f.Tick();
  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(T{{0}}));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(S{}));

  f.Tick();
  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(T{}));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(S{}));

  f.Tick();
  EXPECT_THAT(f.Get<0>().Get(), UnorderedElementsAreArray(T{{0}}));
  EXPECT_THAT(f.Get<1>().Get(), UnorderedElementsAreArray(S{}));
}

TEST(FluentExecutor, SimpleCommunication) {
  auto reroute = [](const std::string& s) {
    return [s](const std::tuple<std::string, int>& t) {
      return std::make_tuple(s, std::get<1>(t));
    };
  };

  zmq::context_t context(1);
  postgres::ConnectionConfig connection_config;
  auto ping =
      noopfluent("name", "inproc://ping", &context, connection_config)
          .channel<std::string, int>("c")
          .RegisterRules([&reroute](auto& c) {
            using namespace fluent::infix;
            return std::make_tuple(
                c <= (c.Iterable() | ra::map(reroute("inproc://pong"))));
          });
  auto pong =
      noopfluent("name", "inproc://pong", &context, connection_config)
          .channel<std::string, int>("c")
          .RegisterRules([&reroute](auto& c) {
            using namespace fluent::infix;
            return std::make_tuple(
                c <= (c.Iterable() | ra::map(reroute("inproc://ping"))));
          });

  using C = std::set<std::tuple<std::string, int>>;
  C catalyst = {{"inproc://pong", 42}};
  ping.MutableGet<0>().Merge(catalyst, 9001);

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
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
