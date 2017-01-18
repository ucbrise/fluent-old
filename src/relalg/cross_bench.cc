#include "relalg/cross.h"

#include <vector>

#include "benchmark/benchmark.h"
#include "range/v3/all.hpp"

namespace fluent {

void ManualSimpleBench(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::vector<int> xs(state.range_x());
    std::vector<int> ys(state.range_x());
    state.ResumeTiming();
    for (int x : xs) {
      for (int y : ys) {
        benchmark::DoNotOptimize(x);
        benchmark::DoNotOptimize(y);
      }
    }
  }
}
BENCHMARK(ManualSimpleBench)->Range(10, 10 << 5);

void CrossSimpleBench(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::vector<int> xs(state.range_x());
    std::vector<int> ys(state.range_x());
    state.ResumeTiming();
    auto crossed = cross(ranges::view::all(xs), ranges::view::all(ys));
    for (auto i = ranges::begin(crossed); i != ranges::end(crossed); ++i) {
      benchmark::DoNotOptimize(*i);
    }
  }
}
BENCHMARK(CrossSimpleBench)->Range(10, 10 << 5);

}  // namespace fluent

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
