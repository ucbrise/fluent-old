#ifndef COMMON_CONCURRENT_QUEUE_H_
#define COMMON_CONCURRENT_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <vector>

#include "common/macros.h"

namespace fluent {
namespace common {

template <typename T>
class ConcurrentQueue {
 public:
  ConcurrentQueue() {}
  explicit ConcurrentQueue(std::vector<T> xs) : xs_(std::move(xs)) {}
  DISALLOW_COPY_AND_ASSIGN(ConcurrentQueue);
  DEFAULT_MOVE_AND_ASSIGN(ConcurrentQueue);

  void Push(const T& x) {
    std::unique_lock<std::mutex> l(m_);
    xs_.push_back(x);
    data_available_.notify_one();
  }

  T Pop() {
    std::unique_lock<std::mutex> l(m_);
    while (xs_.size() == 0) {
      data_available_.wait(l);
    }
    const T x = xs_.front();
    xs_.erase(std::begin(xs_));
    return x;
  }

 private:
  std::mutex m_;
  std::condition_variable data_available_;
  std::vector<T> xs_;
};

}  // namespace common
}  // namespace fluent

#endif  // COMMON_CONCURRENT_QUEUE_H_
