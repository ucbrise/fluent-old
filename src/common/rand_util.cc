#include "common/rand_util.h"

#include "glog/logging.h"

namespace fluent {
namespace common {

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

std::discrete_distribution<int> ZipfDistribution(int n, float alpha) {
  std::vector<double> weights;
  for (int i = 0; i < n; ++i) {
    double weight = 1.0 / std::pow(i + 1, alpha);
    CHECK_NE(0, weight);
    weights.push_back(weight);
  }
  return std::discrete_distribution<int>(weights.begin(), weights.end());
}

}  // namespace common
}  // namespace fluent
