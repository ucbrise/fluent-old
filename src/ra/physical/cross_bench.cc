#include "ra/physical/cross.h"

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

void CrossBaselineBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t>> xs(state.range(0));
  for (std::size_t i = 0; i < xs.size(); ++i) {
    xs[i] = {i};
  }
  std::vector<std::tuple<std::size_t>> ys(state.range(0));
  for (std::size_t i = 0; i < ys.size(); ++i) {
    ys[i] = {state.range(0) + i};
  }

  while (state.KeepRunning()) {
    for (const std::tuple<std::size_t>& x : xs) {
      for (const std::tuple<std::size_t>& y : ys) {
        benchmark::DoNotOptimize(std::tuple_cat(x, y));
      }
    }
  }
}
BENCHMARK(CrossBaselineBench)->Arg(10 << 5);

void CrossBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t>> xs(state.range(0));
  for (std::size_t i = 0; i < xs.size(); ++i) {
    xs[i] = {i};
  }
  std::vector<std::tuple<std::size_t>> ys(state.range(0));
  for (std::size_t i = 0; i < ys.size(); ++i) {
    ys[i] = {state.range(0) + i};
  }

  while (state.KeepRunning()) {
    auto xs_iter = pra::make_iterable(&xs);
    auto ys_iter = pra::make_iterable(&ys);
    auto cross = pra::make_cross(std::move(xs_iter), std::move(ys_iter));
    auto rng = cross.ToRange();
    ranges::for_each(rng, [](const std::tuple<std::size_t, std::size_t>& t) {
      benchmark::DoNotOptimize(t);
    });
  }
}
BENCHMARK(CrossBench)->Arg(10 << 5);

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
