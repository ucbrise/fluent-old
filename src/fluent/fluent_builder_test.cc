#include "fluent/fluent_builder.h"

#include <cstddef>

#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "ra/all.h"

namespace fluent {

TEST(FluentBuilder, SimpleBuildCheck) {
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
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
