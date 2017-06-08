#include "ra/physical/filter.h"

#include <cstddef>

#include <string>
#include <tuple>
#include <vector>

#include "benchmark/benchmark.h"
#include "glog/logging.h"
#include "range/v3/all.hpp"

#include "ra/physical/iterable.h"

namespace pra = fluent::ra::physical;

namespace fluent {
namespace {

bool both_even(const std::tuple<std::size_t, std::size_t>& t) {
  std::size_t x = std::get<0>(t);
  std::size_t y = std::get<1>(t);
  return x % 2 == 0 && y % 2 == 0;
}

}  // namespace

void FilterBaselineBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i};
  }

  while (state.KeepRunning()) {
    for (const std::tuple<std::size_t, std::size_t>& t : ts) {
      if (both_even(t)) {
        benchmark::DoNotOptimize(t);
      }
    }
  }
}
BENCHMARK(FilterBaselineBench)->Arg(10 << 10);

void FilterBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i};
  }

  while (state.KeepRunning()) {
    auto iter = pra::make_iterable(&ts);
    auto filter = pra::make_filter(std::move(iter), both_even);
    auto rng = filter.ToRange();
    ranges::for_each(rng, [](const std::tuple<std::size_t, std::size_t>& t) {
      benchmark::DoNotOptimize(t);
    });
  }
}
BENCHMARK(FilterBench)->Arg(10 << 10);

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
