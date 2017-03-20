#include "common/hash_util.h"

#include <cstddef>

#include <functional>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(Hash, HashBuiltins) {
  EXPECT_EQ(Hash<bool>()(true), std::hash<bool>()(true));
  EXPECT_EQ(Hash<char>()('a'), std::hash<char>()('a'));
  EXPECT_EQ(Hash<int>()(42), std::hash<int>()(42));
}

TEST(Hash, TupleHash) {
  std::tuple<int, char, bool> x = {0, '1', true};
  std::tuple<int, char, bool> y = {0, '2', true};
  std::size_t hash_x = Hash<decltype(x)>()(x);
  std::size_t hash_y = Hash<decltype(y)>()(y);

  // It's too hard to compute hashes by hand, but we can assert that two
  // different tuples should hash to different values. It is possible that they
  // hash to the same value, but the odds should be really really low.
  EXPECT_NE(hash_x, hash_y);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
