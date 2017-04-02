#include "ra/cross.h"

#include <set>
#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/type_list.h"
#include "ra/iterable.h"
#include "ra/project.h"
#include "testing/test_util.h"

namespace fluent {

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

TEST(Cross, MultipleDerivations) {
  Hash<std::tuple<int, int>> hash_xs;
  std::tuple<int, int> xs1 = {1, 1};

  Hash<std::tuple<int, int>> hash_ys;
  std::tuple<int, int> ysa = {2, 3};
  std::tuple<int, int> ysb = {3, 3};

  std::set<std::tuple<int, int>> xs = {xs1};
  std::set<std::tuple<int, int>> ys = {ysa, ysb};

  auto joined = ra::make_cross(ra::make_iterable("xs", &xs),
                               ra::make_iterable("ys", &ys)) |
                ra::project<0, 3>();

  std::set<ra::LineagedTuple<int, int>> expected = {
      ra::make_lineaged_tuple({{"xs", hash_xs(xs1)}, {"ys", hash_ys(ysa)}},
                              std::tuple<int, int>(1, 3)),
      ra::make_lineaged_tuple({{"xs", hash_xs(xs1)}, {"ys", hash_ys(ysb)}},
                              std::tuple<int, int>(1, 3)),
  };

  static_assert(
      std::is_same<decltype(joined)::column_types, TypeList<int, int>>::value,
      "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
  EXPECT_EQ(joined.ToDebugString(), "Project<0, 3>(Cross(xs, ys))");
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
