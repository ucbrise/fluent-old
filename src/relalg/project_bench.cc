#include "relalg/project.h"

#include <tuple>
#include <utility>
#include <vector>

#include "benchmark/benchmark.h"
#include "range/v3/all.hpp"

namespace fluent {

void ManualFirstAndThirdBench(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::vector<std::tuple<int, bool, float>> xs(state.range_x());
    for (int i = 0; i < state.range_x(); ++i) {
      xs[i] = {i, i % 2 == 0, i};
    }
    state.ResumeTiming();
    for (const auto& t : xs) {
      benchmark::DoNotOptimize(std::make_tuple(std::get<0>(t), std::get<2>(t)));
    }
  }
}
BENCHMARK(ManualFirstAndThirdBench)->Range(10, 10 << 10);

void ProjectFirstAndThirdBench(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::vector<std::tuple<int, bool, float>> xs(state.range_x());
    for (int i = 0; i < state.range_x(); ++i) {
      xs[i] = {i, i % 2 == 0, i};
    }
    state.ResumeTiming();
    auto projected = xs | project([](const auto& t) {
                       return std::make_tuple(std::get<0>(t), std::get<2>(t));
                     });
    ranges::for_each(projected,
                     [](const auto& t) { benchmark::DoNotOptimize(t); });
  }
}
BENCHMARK(ProjectFirstAndThirdBench)->Range(10, 10 << 10);

}  // namespace fluent

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
