#include <chrono>
#include <iostream>

#include "glog/logging.h"
#include "redox.hpp"

int main() {
  // Connect to redis.
  redox::Redox rdx;
  rdx.noWait(true);
  if (!rdx.connect("localhost", 6379)) {
    return 1;
  }

  std::cout << "Sending SET synchronously for 5 seconds." << std::endl;

  using namespace std::chrono;
  nanoseconds duration = seconds(5);
  time_point<system_clock> start = system_clock::now();
  time_point<system_clock> stop = start + duration;
  int count = 1;
  for (count = 1; system_clock::now() < stop; ++count) {
    rdx.set("k", "1");
  }

  nanoseconds elapsed = system_clock::now() - start;
  double seconds = elapsed.count() / 1e9;
  double frequency = static_cast<double>(count - 1) / seconds;
  std::cout << (count - 1) << " commands" << std::endl;
  std::cout << seconds << "seconds" << std::endl;
  std::cout << frequency << " commands/second" << std::endl;

  rdx.disconnect();
  return 0;
}
