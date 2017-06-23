#include "common/rand_util.h"

namespace fluent {

int RandInt(int low, int high) {
  std::random_device random_device;
  std::mt19937 random_engine(random_device());
  std::uniform_int_distribution<int> distribution(low, high - 1);
  return distribution(random_engine);
}

std::string RandomAlphanum(int len) {
  static const std::string chars =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  std::random_device random_device;
  std::mt19937 random_engine(random_device());
  std::uniform_int_distribution<int> distribution(0, chars.size() - 1);

  std::string s;
  for (int i = 0; i < len; ++i) {
    s += chars[distribution(random_engine)];
  }
  return s;
}

}  // namespace fluent
