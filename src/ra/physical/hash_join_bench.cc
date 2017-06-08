#include "ra/physical/hash_join.h"

#include <cstddef>

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "benchmark/benchmark.h"
#include "glog/logging.h"
#include "range/v3/all.hpp"

#include "ra/physical/iterable.h"

namespace pra = fluent::ra::physical;
namespace ra = fluent::ra;

namespace fluent {

void HashJoinBaselineBench(benchmark::State& state) {
  std::map<std::size_t, std::vector<std::tuple<std::size_t>>> left;
  for (std::size_t i = 0; i < static_cast<std::size_t>(state.range(0)); ++i) {
    left[i].push_back(std::tuple<std::size_t>(i));
  }
  std::vector<std::tuple<std::size_t, std::size_t>> right(state.range(0));
  for (std::size_t i = 0; i < right.size(); ++i) {
    right[i] = {i, i};
  }

  while (state.KeepRunning()) {
    for (const std::tuple<std::size_t, std::size_t>& r : right) {
      for (const std::tuple<std::size_t>& l : left[std::get<0>(r)]) {
        benchmark::DoNotOptimize(std::tuple_cat(l, r));
      }
    }
  }
}
BENCHMARK(HashJoinBaselineBench)->Arg(10 << 5);

void HashJoinBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t>> xs(state.range(0));
  for (std::size_t i = 0; i < xs.size(); ++i) {
    xs[i] = {i};
  }
  std::vector<std::tuple<std::size_t, std::size_t>> ys(state.range(0));
  for (std::size_t i = 0; i < ys.size(); ++i) {
    ys[i] = {i, i};
  }

  while (state.KeepRunning()) {
    auto xs_iter = pra::make_iterable(&xs);
    auto ys_iter = pra::make_iterable(&ys);
    using left_keys = ra::LeftKeys<0>;
    using right_keys = ra::RightKeys<0>;
    using cols = std::tuple<std::size_t>;
    using keys = std::tuple<std::size_t>;
    auto hash_join = pra::make_hash_join<left_keys, right_keys, cols, keys>(
        std::move(xs_iter), std::move(ys_iter));
    auto rng = hash_join.ToRange();
    ranges::for_each(
        rng, [](const std::tuple<std::size_t, std::size_t, std::size_t>& t) {
          benchmark::DoNotOptimize(t);
        });
  }
}
BENCHMARK(HashJoinBench)->Arg(10 << 5);

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
