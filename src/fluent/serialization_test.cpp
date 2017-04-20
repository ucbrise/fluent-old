#include "fluent/serialization.h"

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "range/v3/all.hpp"

#include "common/type_list.h"
#include "testing/test_util.h"

namespace fluent {

TEST(Serialization, SimpleTest) {
  std::set<std::tuple<int, std::string>> xs = {{1, "A"}, {2, "B"}, {3, "C"}};
  //std::set<std::tuple<int, std::string>> ys = ToString(xs);
  std::string bytestring = ToString(xs);
  //std::cout << bytestring << "\n";
  std::set<std::tuple<int, std::string>> ys = FromString<std::set<std::tuple<int, std::string>>>(bytestring);
  ExpectRngsUnorderedEqual(ys, xs);
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
