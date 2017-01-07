#include "example/count.h"

#include <vector>

#include "benchmark/benchmark.h"
#include "range/v3/all.hpp"

namespace example {

void CountBench(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::vector<int> xs(state.range_x());
    state.ResumeTiming();
    benchmark::DoNotOptimize(Count(ranges::view::all(xs)));
  }
}
BENCHMARK(CountBench)->Range(10, 10 << 10);

}  // namespace example

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
