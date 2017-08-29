#include "ra/physical/group_by.h"

#include <cstddef>

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "benchmark/benchmark.h"
#include "glog/logging.h"
#include "range/v3/all.hpp"

#include "common/sizet_list.h"
#include "common/type_list.h"
#include "ra/aggregates.h"
#include "ra/physical/iterable.h"

namespace pra = fluent::ra::physical;
namespace ra = fluent::ra;

namespace fluent {

void GroupByBaselineBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i};
  }

  while (state.KeepRunning()) {
    std::map<std::size_t, std::size_t> sums;
    for (const std::tuple<std::size_t, std::size_t>& t : ts) {
      sums[std::get<0>(t)] += std::get<1>(t);
    }

    for (const std::pair<std::size_t, std::size_t>& sum : sums) {
      benchmark::DoNotOptimize(sum);
    }
  }
}
BENCHMARK(GroupByBaselineBench)->Arg(10 << 10);

void GroupByBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i};
  }

  while (state.KeepRunning()) {
    auto iter = pra::make_iterable(&ts);
    using keys = ra::Keys<0>;
    using key_cols = std::tuple<std::size_t>;
    using aggs = std::tuple<
        ra::agg::SumImpl<common::SizetList<1>, common::TypeList<std::size_t>>>;
    auto group_by = pra::make_group_by<keys, key_cols, aggs>(std::move(iter));
    auto rng = group_by.ToRange();
    ranges::for_each(rng, [](const std::tuple<std::size_t, std::size_t>& t) {
      benchmark::DoNotOptimize(t);
    });
  }
}
BENCHMARK(GroupByBench)->Arg(10 << 10);

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
