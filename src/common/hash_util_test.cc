#include "common/hash_util.h"

#include <chrono>
#include <cstddef>
#include <functional>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "testing/mock_clock.h"

namespace fluent {

TEST(Hash, HashBuiltins) {
  EXPECT_EQ(Hash<bool>()(true), std::hash<bool>()(true));
  EXPECT_EQ(Hash<char>()('a'), std::hash<char>()('a'));
  EXPECT_EQ(Hash<int>()(42), std::hash<int>()(42));
}

TEST(Hash, TimePointHash) {
  Hash<std::chrono::time_point<MockClock>> hash;
  std::chrono::time_point<MockClock> x(std::chrono::seconds(1));
  std::chrono::time_point<MockClock> y(std::chrono::seconds(2));
  std::chrono::time_point<MockClock> z(std::chrono::milliseconds(1000));

  EXPECT_EQ(hash(x), hash(x));
  EXPECT_EQ(hash(y), hash(y));
  EXPECT_EQ(hash(z), hash(z));
  EXPECT_EQ(hash(x), hash(z));
  EXPECT_NE(hash(x), hash(y));
}

TEST(Hash, VectorHash) {
  std::vector<int> w = {};
  std::vector<int> x = {1};
  std::vector<int> y = {1, 2};
  std::vector<int> z = {1};
  std::size_t hash_w = Hash<std::vector<int>>()(w);
  std::size_t hash_x = Hash<std::vector<int>>()(x);
  std::size_t hash_y = Hash<std::vector<int>>()(y);
  std::size_t hash_z = Hash<std::vector<int>>()(z);

  // It's too hard to compute hashes by hand, but we can assert that two
  // different vectors should hash to different values. It is possible that
  // they hash to the same value, but the odds should be really really low.
  EXPECT_NE(hash_w, hash_x);
  EXPECT_NE(hash_w, hash_y);
  EXPECT_NE(hash_w, hash_z);
  EXPECT_NE(hash_x, hash_y);

  // We can also check that two equal vectors hash to the same value.
  EXPECT_EQ(hash_x, hash_z);
}

TEST(Hash, TupleHash) {
  std::tuple<int, char, bool> x = {0, '1', true};
  std::tuple<int, char, bool> y = {0, '2', true};
  std::size_t hash_x = Hash<decltype(x)>()(x);
  std::size_t hash_y = Hash<decltype(y)>()(y);

  // See VectorHash.
  EXPECT_NE(hash_x, hash_y);
  std::tuple<int, char, bool> z = {0, '1', true};
  std::size_t hash_z = Hash<decltype(z)>()(z);
  EXPECT_EQ(hash_x, hash_z);
}

TEST(Hash, TupleTupleHash) {
  std::tuple<std::tuple<int, char>, std::tuple<bool>> x = {{0, '1'}, {true}};
  std::tuple<std::tuple<int, char>, std::tuple<bool>> y = {{0, '2'}, {true}};
  std::tuple<std::tuple<int, char>, std::tuple<bool>> z = {{0, '1'}, {true}};
  std::size_t hash_x = Hash<decltype(x)>()(x);
  std::size_t hash_y = Hash<decltype(y)>()(y);
  std::size_t hash_z = Hash<decltype(z)>()(z);

  EXPECT_NE(hash_x, hash_y);
  EXPECT_EQ(hash_x, hash_z);
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
