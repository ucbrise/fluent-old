#include "ra/all.h"

#include <set>
#include <tuple>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/hash_util.h"
#include "common/type_list.h"
#include "ra/lineaged_tuple.h"
#include "testing/test_util.h"

namespace agg = fluent::ra::agg;

namespace fluent {

TEST(All, AllOperatorsExceptCount) {
  Hash<std::tuple<int>> hash_xs;
  std::tuple<int> xs0 = {10};
  std::tuple<int> xs1 = {20};
  std::tuple<int> xs2 = {30};
  std::set<std::tuple<int>> xs = {xs0, xs1, xs2};

  Hash<std::tuple<int>> hash_ys;
  std::tuple<int> ys0 = {2};
  std::tuple<int> ys1 = {3};
  std::tuple<int> ys2 = {4};
  std::set<std::tuple<int>> ys = {ys0, ys1, ys2};

  Hash<std::tuple<int, int, int>> hash_zs;
  std::tuple<int, int, int> zs0 = {6, 100, 100};
  std::tuple<int, int, int> zs1 = {6, 200, 200};
  std::tuple<int, int, int> zs2 = {7, 300, 300};
  std::set<std::tuple<int, int, int>> zs = {zs0, zs1, zs2};

  auto times_two = [](const std::tuple<int>& x) -> std::tuple<int> {
    return {std::get<0>(x) * 2};
  };
  auto is_even = [](const std::tuple<int>& x) {
    return std::get<0>(x) % 2 == 0;
  };

  auto mapped = ra::make_iterable("xs", &xs) | ra::map(times_two);
  auto filtered = ra::make_iterable("ys", &ys) | ra::filter(is_even);
  auto crossed = ra::make_cross(std::move(mapped), std::move(filtered));
  auto grouped = std::move(crossed) | ra::group_by<ra::Keys<0>, agg::Sum<1>>();
  auto projected = ra::make_iterable("zs", &zs) | ra::project<0, 1>();
  auto joined = ra::make_hash_join<ra::LeftKeys<1>, ra::RightKeys<0>>(
      std::move(grouped), std::move(projected));

  using Tuple = std::tuple<int, int, int, int>;
  std::set<ra::LineagedTuple<int, int, int, int>> expected = {
      ra::make_lineaged_tuple(
          {
              {"xs", hash_xs(xs0)},
              {"ys", hash_ys(ys0)},
              {"ys", hash_ys(ys2)},
              {"zs", hash_zs(zs0)},
          },
          Tuple{20, 6, 6, 100}),
      ra::make_lineaged_tuple(
          {
              {"xs", hash_xs(xs0)},
              {"ys", hash_ys(ys0)},
              {"ys", hash_ys(ys2)},
              {"zs", hash_zs(zs1)},
          },
          Tuple{20, 6, 6, 200}),
      ra::make_lineaged_tuple(
          {
              {"xs", hash_xs(xs1)},
              {"ys", hash_ys(ys0)},
              {"ys", hash_ys(ys2)},
              {"zs", hash_zs(zs0)},
          },
          Tuple{40, 6, 6, 100}),
      ra::make_lineaged_tuple(
          {
              {"xs", hash_xs(xs1)},
              {"ys", hash_ys(ys0)},
              {"ys", hash_ys(ys2)},
              {"zs", hash_zs(zs1)},
          },
          Tuple{40, 6, 6, 200}),
      ra::make_lineaged_tuple(
          {
              {"xs", hash_xs(xs2)},
              {"ys", hash_ys(ys0)},
              {"ys", hash_ys(ys2)},
              {"zs", hash_zs(zs0)},
          },
          Tuple{60, 6, 6, 100}),
      ra::make_lineaged_tuple(
          {
              {"xs", hash_xs(xs2)},
              {"ys", hash_ys(ys0)},
              {"ys", hash_ys(ys2)},
              {"zs", hash_zs(zs1)},
          },
          Tuple{60, 6, 6, 200}),
  };

  static_assert(std::is_same<decltype(joined)::column_types,
                             TypeList<int, int, int, int>>::value,
                "");
  ExpectRngsUnorderedEqual(joined.ToPhysical().ToRange(), expected);
  EXPECT_EQ(joined.ToDebugString(),
            "HashJoin<LeftKeys<1>, RightKeys<0>>("
            "GroupBy<Keys<0>, Sum<1>>(Cross(Map(xs), Filter(ys))), "
            "Project<0, 1>(zs)"
            ")");
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
