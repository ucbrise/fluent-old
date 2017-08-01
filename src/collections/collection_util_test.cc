#include "collections/collection_util.h"

#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common/mock_pickler.h"
#include "testing/mock_clock.h"

namespace fluent {
namespace collections {

using common::MockPickler;
using testing::MockClock;

TEST(CollectionUtil, CollectionTypes) {
  // Tables.
  static_assert(std::is_same<                        //
                    CollectionTypes<Table<>>::type,  //
                    common::TypeList<>>::value,      //
                "");
  static_assert(std::is_same<                           //
                    CollectionTypes<Table<int>>::type,  //
                    common::TypeList<int>>::value,      //
                "");
  static_assert(std::is_same<                                       //
                    CollectionTypes<Table<int, char, bool>>::type,  //
                    common::TypeList<int, char, bool>>::value,      //
                "");

  // Scratches.
  static_assert(std::is_same<                          //
                    CollectionTypes<Scratch<>>::type,  //
                    common::TypeList<>>::value,        //
                "");
  static_assert(std::is_same<                             //
                    CollectionTypes<Scratch<int>>::type,  //
                    common::TypeList<int>>::value,        //
                "");
  static_assert(std::is_same<                                         //
                    CollectionTypes<Scratch<int, char, bool>>::type,  //
                    common::TypeList<int, char, bool>>::value,        //
                "");

  // Channels.
  static_assert(
      std::is_same<                                                  //
          CollectionTypes<Channel<MockPickler, std::string>>::type,  //
          common::TypeList<std::string>>::value,                     //
      "");
  static_assert(
      std::is_same<                                                       //
          CollectionTypes<Channel<MockPickler, std::string, int>>::type,  //
          common::TypeList<std::string, int>>::value,                     //
      "");
  static_assert(
      std::is_same<  //
          CollectionTypes<
              Channel<MockPickler, std::string, int, char, bool>>::type,  //
          common::TypeList<std::string, int, char, bool>>::value,         //
      "");

  // Stdin.
  static_assert(std::is_same<                               //
                    CollectionTypes<Stdin>::type,           //
                    common::TypeList<std::string>>::value,  //
                "");

  // Stdout.
  static_assert(std::is_same<                               //
                    CollectionTypes<Stdout>::type,          //
                    common::TypeList<std::string>>::value,  //
                "");

  // Periodic.
  static_assert(std::is_same<                                    //
                    CollectionTypes<Periodic<MockClock>>::type,  //
                    common::TypeList<Periodic<MockClock>::id,
                                     Periodic<MockClock>::time>>::value,  //
                "");
}

TEST(CollectionUtil, GetCollectionType) {
  EXPECT_EQ(CollectionType::TABLE, (GetCollectionType<Table<>>::value));
  EXPECT_EQ(CollectionType::TABLE, (GetCollectionType<Table<int>>::value));
  EXPECT_EQ(CollectionType::TABLE, (GetCollectionType<Table<int, int>>::value));

  EXPECT_EQ(CollectionType::SCRATCH, (GetCollectionType<Scratch<>>::value));
  EXPECT_EQ(CollectionType::SCRATCH, (GetCollectionType<Scratch<int>>::value));
  EXPECT_EQ(CollectionType::SCRATCH,
            (GetCollectionType<Scratch<int, int>>::value));

  EXPECT_EQ(CollectionType::CHANNEL,
            (GetCollectionType<Channel<MockPickler, std::string>>::value));
  EXPECT_EQ(CollectionType::CHANNEL,
            (GetCollectionType<Channel<MockPickler, std::string, int>>::value));

  EXPECT_EQ(CollectionType::STDIN, (GetCollectionType<Stdin>::value));
  EXPECT_EQ(CollectionType::STDOUT, (GetCollectionType<Stdout>::value));
  EXPECT_EQ(CollectionType::PERIODIC,
            (GetCollectionType<Periodic<MockClock>>::value));
}

TEST(CollectionUtil, CollectionTypeToString) {
  auto to_string = CollectionTypeToString;
  EXPECT_STREQ("Table", to_string(CollectionType::TABLE).c_str());
  EXPECT_STREQ("Scratch", to_string(CollectionType::SCRATCH).c_str());
  EXPECT_STREQ("Channel", to_string(CollectionType::CHANNEL).c_str());
  EXPECT_STREQ("Stdin", to_string(CollectionType::STDIN).c_str());
  EXPECT_STREQ("Stdout", to_string(CollectionType::STDOUT).c_str());
  EXPECT_STREQ("Periodic", to_string(CollectionType::PERIODIC).c_str());
}

}  // namespace collections
}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
