#include "ra/physical/iterable.h"

#include <cstddef>

#include <tuple>
#include <vector>

#include "benchmark/benchmark.h"
#include "glog/logging.h"
#include "range/v3/all.hpp"

namespace pra = fluent::ra::physical;

namespace fluent {

void IterableBaselineBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i};
  }

  while (state.KeepRunning()) {
    for (const std::tuple<std::size_t, std::size_t>& t : ts) {
      benchmark::DoNotOptimize(t);
    }
  }
}
BENCHMARK(IterableBaselineBench)->Arg(10 << 10);

void IterableBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i};
  }

  while (state.KeepRunning()) {
    auto iter = pra::make_iterable(&ts);
    auto rng = iter.ToRange();
    ranges::for_each(rng, [](const std::tuple<std::size_t, std::size_t>& t) {
      benchmark::DoNotOptimize(t);
    });
  }
}
BENCHMARK(IterableBench)->Arg(10 << 10);

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
