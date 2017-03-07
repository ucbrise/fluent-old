#include "ra/ra_util.h"

#include <set>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ra/iterable.h"

namespace fluent {

TEST(RaUtil, SimpleRa) {
  std::vector<std::tuple<int, char>> xs = {
      {1, '1'}, {2, '2'}, {3, '3'}, {4, '4'}};
  auto ra = ra::make_iterable(&xs);

  std::set<std::tuple<int, char>> buffered;
  std::vector<std::tuple<int, char>> streamed;
  ra::BufferRaInto(ra, &buffered);
  ra::StreamRaInto(ra, &streamed);

  EXPECT_THAT(buffered, testing::UnorderedElementsAreArray(xs));
  EXPECT_THAT(streamed, testing::UnorderedElementsAreArray(xs));
}

}  // namespace fluent

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
