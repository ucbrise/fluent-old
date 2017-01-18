#include "relalg/select.h"

#include <vector>

#include "benchmark/benchmark.h"
#include "range/v3/all.hpp"

namespace fluent {

namespace detail {

bool IsPrime(const int x) {
  if (x < 2) {
    return false;
  }
  for (int i = 2; i < x; ++i) {
    if (x % i == 0) {
      return false;
    }
  }
  return true;
}

}  // namespace

void ManualAllBench(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::vector<int> xs(state.range_x());
    state.ResumeTiming();
    for (int x : xs) {
      benchmark::DoNotOptimize(x);
    }
  }
}
BENCHMARK(ManualAllBench)->Range(10, 10 << 10);

void SelectAllBench(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::vector<int> xs(state.range_x());
    state.ResumeTiming();
    auto selected = xs | select([](int) { return true; });
    ranges::for_each(selected, [](int x) { benchmark::DoNotOptimize(x); });
  }
}
BENCHMARK(SelectAllBench)->Range(10, 10 << 10);

void ManualEvensBench(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::vector<int> xs(state.range_x());
    state.ResumeTiming();
    for (int x : xs) {
      if (detail::IsPrime(x)) {
        benchmark::DoNotOptimize(x);
      } else {
        benchmark::DoNotOptimize(x);
      }
    }
  }
}
BENCHMARK(ManualEvensBench)->Range(10, 10 << 10);

void SelectEvensBench(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::vector<int> xs(state.range_x());
    state.ResumeTiming();
    auto selected = xs | select(detail::IsPrime);
    ranges::for_each(selected, [](int x) { benchmark::DoNotOptimize(x); });
  }
}
BENCHMARK(SelectEvensBench)->Range(10, 10 << 10);

void ManualPrimesBench(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::vector<int> xs(state.range_x());
    state.ResumeTiming();
    for (int x : xs) {
      if (x % 2 == 0) {
        benchmark::DoNotOptimize(x);
      } else {
        benchmark::DoNotOptimize(x);
      }
    }
  }
}
BENCHMARK(ManualPrimesBench)->Range(10, 10 << 10);

void SelectPrimesBench(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::vector<int> xs(state.range_x());
    state.ResumeTiming();
    auto selected = xs | select([](int x) { return x % 2 == 0; });
    ranges::for_each(selected, [](int x) { benchmark::DoNotOptimize(x); });
  }
}
BENCHMARK(SelectPrimesBench)->Range(10, 10 << 10);

}  // namespace fluent

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
