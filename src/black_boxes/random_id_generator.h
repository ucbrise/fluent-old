#ifndef BLACK_BOXES_RANDOM_ID_GENERATOR_H_
#define BLACK_BOXES_RANDOM_ID_GENERATOR_H_

#include <cstdint>

#include <limits>
#include <random>

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

#endif  // BLACK_BOXES_RANDOM_ID_GENERATOR_H_
