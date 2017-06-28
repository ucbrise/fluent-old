#ifndef FLUENT_FLUENT_TIMESTAMP_WRAPPER_H_
#define FLUENT_FLUENT_TIMESTAMP_WRAPPER_H_

namespace fluent {

class TimestampWrapper {
 public:
  TimestampWrapper() : x_(nullptr) {}

  void Set(int* x) { x_ = x; }

  int Get() const { return *x_; }

 private:
  int* x_;
};

}  // namespace fluent

#endif  // FLUENT_FLUENT_TIMESTAMP_WRAPPER_H_
