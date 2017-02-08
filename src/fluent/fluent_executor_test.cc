#include "fluent/fluent_executor.h"

#include <cstddef>

#include <tuple>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "fluent/collection_builder.h"
#include "ra/all.h"

using ::testing::UnorderedElementsAreArray;

namespace fluent {

TEST(FluentExecutor, SimpleProgram) {
  // clang-format off
  auto f = fluent("inproc://yolo")
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
  // clang-format off
  auto f = fluent("inproc://yolo")
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
  // clang-format off
  auto f = fluent("inproc://yolo")
    .table<int>("t")
    .RegisterRules([](auto& t) { return std::make_tuple(t <= t.Iterable()); });
  // clang-format on

  // TODO(mwhittaker): Add a test in which two fluent programs communicate with
  // one another using channels.
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
