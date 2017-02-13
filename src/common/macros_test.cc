#include "common/macros.h"

#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {

class CopyableAndAssignable {
 public:
  CopyableAndAssignable(std::vector<int> xs) : xs_(std::move(xs)) {}

 private:
  std::vector<int> xs_;
};

class NotCopyableAndAssignable {
 public:
  NotCopyableAndAssignable(std::vector<int> xs) : xs_(std::move(xs)) {}
  DISALLOW_COPY_AND_ASSIGN(NotCopyableAndAssignable)

 private:
  std::vector<int> xs_;
};

TEST(Macros, DisallowCopyAndAssign) {
  {
    CopyableAndAssignable x({1, 2, 3});
    CopyableAndAssignable y(x);
    y = x;
  }

  // The following code will not compile.
  //   {
  //     NotCopyableAndAssignable x({1, 2, 3});
  //     NotCopyableAndAssignable y(x);
  //     y = x;
  //   }
}

}  // namespace fluent

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
