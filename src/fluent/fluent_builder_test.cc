#include "fluent/fluent_builder.h"

#include <cstddef>

#include <tuple>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "fluent/infix.h"
#include "ra/all.h"

namespace fluent {

TEST(FluentBuilder, SimpleBuildCheck) {
  zmq::context_t context(1);
  // clang-format off
  auto f = fluent("inproc://yolo", &context)
    .table<std::string, int>("t")
    .scratch<std::string, int>("s")
    .channel<std::string, int>("c")
    .stdout()
    .RegisterRules([](auto& t, auto& s, auto& c, auto& out) {
      using namespace fluent::infix;
      auto rule_a = t <= s.Iterable();
      auto rule_b = t += s.Iterable();
      auto rule_c = t -= s.Iterable();
      auto rule_d = s <= s.Iterable();
      auto rule_e = c <= s.Iterable();
      auto rule_f = out <= (s.Iterable() | ra::project<0>());
      auto rule_g = out += (s.Iterable() | ra::project<0>());
      return std::make_tuple(rule_a, rule_b, rule_c, rule_d, rule_e,
                             rule_f, rule_g);
    });
  // clang-format on
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
