#include "ra/physical/flat_map.h"

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

std::vector<std::tuple<std::size_t>> range(
    const std::tuple<std::size_t, std::size_t>& t) {
  std::vector<std::tuple<std::size_t>> xs;
  for (std::size_t i = std::get<0>(t); i < std::get<1>(t); ++i) {
    xs.push_back(std::tuple<std::size_t>(i));
  }
  return xs;
}

}  // namespace

void FlatMapBaselineBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i + i};
  }

  while (state.KeepRunning()) {
    for (const std::tuple<std::size_t, std::size_t>& t : ts) {
      for (std::size_t i = std::get<0>(t); i < std::get<1>(t); ++i) {
        benchmark::DoNotOptimize(std::tuple<std::size_t>(i));
      }
    }
  }
}
BENCHMARK(FlatMapBaselineBench)->Arg(10 << 5);

void FlatMapBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i + i};
  }

  while (state.KeepRunning()) {
    auto iter = pra::make_iterable(&ts);
    using ret = std::vector<std::tuple<std::size_t>>;
    auto flat_map = pra::make_flat_map<ret>(std::move(iter), range);
    auto rng = flat_map.ToRange();
    ranges::for_each(rng, [](const std::tuple<std::size_t>& t) {
      benchmark::DoNotOptimize(t);
    });
  }
}
BENCHMARK(FlatMapBench)->Arg(10 << 5);

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
