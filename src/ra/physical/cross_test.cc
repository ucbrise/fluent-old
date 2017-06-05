#include "ra/physical/cross.h"

#include <set>
#include <tuple>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "ra/physical/iterable.h"
#include "testing/test_util.h"

namespace pra = fluent::ra::physical;

namespace fluent {

TEST(Cross, EmptyCross) {
  std::set<std::tuple<int>> xs;
  std::set<std::tuple<float>> ys;
  auto iterable_xs = pra::make_iterable(&xs);
  auto iterable_ys = pra::make_iterable(&ys);
  auto cross = pra::make_cross(std::move(iterable_xs), std::move(iterable_ys));
  std::set<std::tuple<int, float>> expected;
  ExpectRngsUnorderedEqual(cross.ToRange(), expected);
}

#if 0
TEST(Cross, SimpleCross) {
  Hash<std::tuple<int>> hash_xs;
  std::tuple<int> xs1 = {1};
  std::tuple<int> xs2 = {2};
  std::tuple<int> xs3 = {3};

  Hash<std::tuple<std::string>> hash_ys;
  std::tuple<std::string> ysa = {"a"};
  std::tuple<std::string> ysb = {"b"};

  std::set<std::tuple<int>> xs = {xs1, xs2, xs3};
  std::set<std::tuple<std::string>> ys = {ysa, ysb};

  auto crossed = ra::make_cross(ra::make_iterable("xs", &xs),
                                ra::make_iterable("ys", &ys));
  std::set<ra::LineagedTuple<int, std::string>> expected = {
      ra::make_lineaged_tuple({{"xs", hash_xs(xs1)}, {"ys", hash_ys(ysa)}},
                              std::tuple<int, std::string>(1, "a")),
      ra::make_lineaged_tuple({{"xs", hash_xs(xs2)}, {"ys", hash_ys(ysa)}},
                              std::tuple<int, std::string>(2, "a")),
      ra::make_lineaged_tuple({{"xs", hash_xs(xs3)}, {"ys", hash_ys(ysa)}},
                              std::tuple<int, std::string>(3, "a")),
      ra::make_lineaged_tuple({{"xs", hash_xs(xs1)}, {"ys", hash_ys(ysb)}},
                              std::tuple<int, std::string>(1, "b")),
      ra::make_lineaged_tuple({{"xs", hash_xs(xs2)}, {"ys", hash_ys(ysb)}},
                              std::tuple<int, std::string>(2, "b")),
      ra::make_lineaged_tuple({{"xs", hash_xs(xs3)}, {"ys", hash_ys(ysb)}},
                              std::tuple<int, std::string>(3, "b")),
  };

  static_assert(std::is_same<decltype(crossed)::column_types,
                             TypeList<int, std::string>>::value,
                "");
  ExpectRngsUnorderedEqual(crossed.ToPhysical().ToRange(), expected);
  EXPECT_EQ(crossed.ToDebugString(), "Cross(xs, ys)");
}
#endif

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
