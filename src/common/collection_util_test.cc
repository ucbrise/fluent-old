#include "common/collection_util.h"

#include <sstream>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(CollectionUtil, SetToString) {
  {
    std::set<int> xs{};
    std::ostringstream os;
    os << xs;
    EXPECT_EQ("{}", os.str());
  }

  {
    std::set<int> xs{1};
    std::ostringstream os;
    os << xs;
    EXPECT_EQ("{1}", os.str());
  }

  {
    std::set<int> xs{1, 2, 3};
    std::ostringstream os;
    os << xs;
    EXPECT_EQ("{1, 2, 3}", os.str());
  }
}

TEST(CollectionUtil, VectorToString) {
  {
    std::vector<int> xs{};
    std::ostringstream os;
    os << xs;
    EXPECT_EQ("[]", os.str());
  }

  {
    std::vector<int> xs{1};
    std::ostringstream os;
    os << xs;
    EXPECT_EQ("[1]", os.str());
  }

  {
    std::vector<int> xs{1, 2, 3};
    std::ostringstream os;
    os << xs;
    EXPECT_EQ("[1, 2, 3]", os.str());
  }
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
