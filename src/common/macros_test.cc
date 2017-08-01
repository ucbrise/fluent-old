#include "common/macros.h"

#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace common {

class CopyableAndAssignable {
 public:
  CopyableAndAssignable() {}
};

class NotCopyableAndAssignable {
 public:
  NotCopyableAndAssignable() {}
  DISALLOW_COPY_AND_ASSIGN(NotCopyableAndAssignable);
};

class NotCopyableAndAssignableButMoveable {
 public:
  NotCopyableAndAssignableButMoveable() {}
  DISALLOW_COPY_AND_ASSIGN(NotCopyableAndAssignableButMoveable);
  DEFAULT_MOVE_AND_ASSIGN(NotCopyableAndAssignableButMoveable);
};

TEST(Macros, DisallowCopyAndAssign) {
  {
    CopyableAndAssignable x;
    CopyableAndAssignable y(x);
    y = x;
  }

  // The following code will not compile.
  //   {
  //     NotCopyableAndAssignable x;
  //     NotCopyableAndAssignable y(x);
  //     y = x;
  //   }
}

TEST(Macros, DefaultMoveAndAssign) {
  NotCopyableAndAssignableButMoveable x;
  NotCopyableAndAssignableButMoveable z(std::move(x));
  z = std::move(x);
}

TEST(Macros, Unused) {
  int x = 0;
  UNUSED(x);
}

}  // namespace common
}  // namespace fluent

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
