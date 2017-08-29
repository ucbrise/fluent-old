#include "common/collection_util.h"

#include <sstream>
#include <string>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {
namespace common {
namespace {

template <typename T>
std::string ToString(const T& x) {
  std::ostringstream os;
  os << x;
  return os.str();
}

}  // namespace

TEST(CollectionUtil, SetToString) {
  EXPECT_EQ("{}", ToString(std::set<int>{}));
  EXPECT_EQ("{1}", ToString(std::set<int>{1}));
  EXPECT_EQ("{1, 2, 3}", ToString(std::set<int>{1, 2, 3}));
}

TEST(CollectionUtil, VectorToString) {
  EXPECT_EQ("[]", ToString(std::vector<int>{}));
  EXPECT_EQ("[1]", ToString(std::vector<int>{1}));
  EXPECT_EQ("[1, 2, 3]", ToString(std::vector<int>{1, 2, 3}));
}

}  // namespace common
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
