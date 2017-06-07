#include "collections/periodic.h"

#include <cstddef>

#include <chrono>
#include <set>
#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "testing/mock_clock.h"

namespace fluent {

TEST(Periodic, PeriodicStartsEmpty) {
  Periodic<MockClock> p("p", std::chrono::milliseconds(1000));
  using id = Periodic<MockClock>::id;
  using time = std::chrono::time_point<MockClock>;
  std::map<std::tuple<id, time>, CollectionTupleIds> expected;
  EXPECT_EQ(p.Get(), expected);
}

TEST(Periodic, GetAndIncrementId) {
  Periodic<MockClock> p("p", std::chrono::milliseconds(1000));
  EXPECT_EQ(0ul, p.GetAndIncrementId());
  EXPECT_EQ(1ul, p.GetAndIncrementId());
  EXPECT_EQ(2ul, p.GetAndIncrementId());
  EXPECT_EQ(3ul, p.GetAndIncrementId());
}

TEST(Periodic, Merge) {
  Periodic<MockClock> p("p", std::chrono::milliseconds(1000));
  using id = Periodic<MockClock>::id;
  using time = std::chrono::time_point<MockClock>;
  std::map<std::tuple<id, time>, CollectionTupleIds> expected;
  time zero(std::chrono::milliseconds(0));
  time one(std::chrono::milliseconds(1));

  p.Merge({0ul, zero}, 0x0, 0);
  expected = {{{0ul, zero}, {0x0, {0}}}};
  EXPECT_EQ(p.Get(), expected);

  p.Merge({1ul, one}, 0x1, 1);
  expected = {{{0ul, zero}, {0x0, {0}}}, {{1ul, one}, {0x1, {1}}}};
  EXPECT_EQ(p.Get(), expected);

  p.Merge({1ul, one}, 0x1, 2);
  expected = {{{0ul, zero}, {0x0, {0}}}, {{1ul, one}, {0x1, {1, 2}}}};
  EXPECT_EQ(p.Get(), expected);
}

TEST(Periodic, TickClearsPeriodic) {
  Periodic<MockClock> p("p", std::chrono::milliseconds(1000));
  using id = Periodic<MockClock>::id;
  using time = std::chrono::time_point<MockClock>;
  std::map<std::tuple<id, time>, CollectionTupleIds> expected;
  time zero(std::chrono::milliseconds(0));
  time one(std::chrono::milliseconds(1));

  p.Merge({0ul, zero}, 0x0, 0);
  expected = {{{0ul, zero}, {0x0, {0}}}};
  EXPECT_EQ(p.Get(), expected);

  p.Tick();
  expected = {};
  EXPECT_EQ(p.Get(), expected);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
