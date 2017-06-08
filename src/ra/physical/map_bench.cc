#include "ra/physical/map.h"

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

std::string tuple_to_string(const std::tuple<std::size_t, std::size_t>& t) {
  std::size_t x = std::get<0>(t);
  std::size_t y = std::get<1>(t);
  return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}

}  // namespace

void MapBaselineBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i};
  }

  while (state.KeepRunning()) {
    for (const std::tuple<std::size_t, std::size_t>& t : ts) {
      benchmark::DoNotOptimize(tuple_to_string(t));
    }
  }
}
BENCHMARK(MapBaselineBench)->Arg(10 << 10);

void MapBench(benchmark::State& state) {
  std::vector<std::tuple<std::size_t, std::size_t>> ts(state.range(0));
  for (std::size_t i = 0; i < ts.size(); ++i) {
    ts[i] = {i, i};
  }

  while (state.KeepRunning()) {
    auto iter = pra::make_iterable(&ts);
    auto map = pra::make_map(std::move(iter), tuple_to_string);
    auto rng = map.ToRange();
    ranges::for_each(rng,
                     [](const std::string& s) { benchmark::DoNotOptimize(s); });
  }
}
BENCHMARK(MapBench)->Arg(10 << 10);

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
