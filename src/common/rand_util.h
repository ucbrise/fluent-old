#ifndef COMMON_RAND_UTIL_H_
#define COMMON_RAND_UTIL_H_

#include <cstdint>

#include <limits>
#include <random>
#include <string>

namespace fluent {

// Returns a random integer in the range [low, high).
int RandInt(int low, int high);

// Returns a random string `xs` of length `len` where each character in `x` is
// one of the following characters:
//
// abcdefghijklmnopqrstuvwxyz
// ABCDEFGHIJKLMNOPQRSTUVWXYZ
// 0123456789
std::string RandomAlphanum(int len);

// Return a zipfian distribution with parameters n and alpha.
std::discrete_distribution<int> ZipfDistribution(int n, float alpha);

// A struct which randomly generates 64 bit unique ids uniformally at random.
//
//   RandomIdGenerator gen;
//   gen.Generate();
//   gen.Generate();
//   ...
class RandomIdGenerator {
 public:
  RandomIdGenerator()
      : random_engine_(random_device_()),
        distribution_(std::numeric_limits<std::int64_t>::lowest(),
                      std::numeric_limits<std::int64_t>::max()) {}

  std::int64_t Generate() { return distribution_(random_engine_); }

 private:
  std::random_device random_device_;
  std::mt19937 random_engine_;
  std::uniform_int_distribution<std::int64_t> distribution_;
};

}  // namespace fluent

#endif  // COMMON_RAND_UTIL_H_
