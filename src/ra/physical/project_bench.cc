#include "ra/physical/project.h"

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

void ProjectBaselineBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i};
  }

  while (state.KeepRunning()) {
    for (const std::tuple<std::size_t, std::size_t>& t : ts) {
      std::tuple<std::size_t> projected = std::make_tuple(std::get<0>(t));
      benchmark::DoNotOptimize(projected);
    }
  }
}
BENCHMARK(ProjectBaselineBench)->Arg(10 << 10);

void ProjectBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i};
  }

  while (state.KeepRunning()) {
    auto iter = pra::make_iterable(&ts);
    auto project = pra::make_project<0>(std::move(iter));
    auto rng = project.ToRange();
    ranges::for_each(rng, [](const std::tuple<std::size_t>& t) {
      benchmark::DoNotOptimize(t);
    });
  }
}
BENCHMARK(ProjectBench)->Arg(10 << 10);

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
