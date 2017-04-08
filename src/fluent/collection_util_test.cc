#include "fluent/collection_util.h"

#include <type_traits>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace fluent {

TEST(CollectionUtil, CollectionTypes) {
  // By default, clang-format formats these static_asserts in a really
  // unreadable way. The weird comments at the end of the lines forces
  // clang-format to format things in a more sane way.

  // Tables.
  static_assert(std::is_same<                        //
                    CollectionTypes<Table<>>::type,  //
                    TypeList<>>::value,              //
                "");
  static_assert(std::is_same<                           //
                    CollectionTypes<Table<int>>::type,  //
                    TypeList<int>>::value,              //
                "");
  static_assert(std::is_same<                                       //
                    CollectionTypes<Table<int, char, bool>>::type,  //
                    TypeList<int, char, bool>>::value,              //
                "");

  // Scratches.
  static_assert(std::is_same<                          //
                    CollectionTypes<Scratch<>>::type,  //
                    TypeList<>>::value,                //
                "");
  static_assert(std::is_same<                             //
                    CollectionTypes<Scratch<int>>::type,  //
                    TypeList<int>>::value,                //
                "");
  static_assert(std::is_same<                                         //
                    CollectionTypes<Scratch<int, char, bool>>::type,  //
                    TypeList<int, char, bool>>::value,                //
                "");

  // Channels.
  static_assert(std::is_same<                                     //
                    CollectionTypes<Channel<std::string>>::type,  //
                    TypeList<std::string>>::value,                //
                "");
  static_assert(std::is_same<                                          //
                    CollectionTypes<Channel<std::string, int>>::type,  //
                    TypeList<std::string, int>>::value,                //
                "");
  static_assert(
      std::is_same<                                                      //
          CollectionTypes<Channel<std::string, int, char, bool>>::type,  //
          TypeList<std::string, int, char, bool>>::value,                //
      "");

  // Stdin.
  static_assert(std::is_same<                       //
                    CollectionTypes<Stdin>::type,   //
                    TypeList<std::string>>::value,  //
                "");

  // Stdout.
  static_assert(std::is_same<                       //
                    CollectionTypes<Stdout>::type,  //
                    TypeList<std::string>>::value,  //
                "");

  // Periodic.
  static_assert(std::is_same<                                        //
                    CollectionTypes<Periodic>::type,                 //
                    TypeList<Periodic::id, Periodic::time>>::value,  //
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
            (GetCollectionType<Channel<std::string>>::value));
  EXPECT_EQ(CollectionType::CHANNEL,
            (GetCollectionType<Channel<std::string, int>>::value));

  EXPECT_EQ(CollectionType::STDIN, (GetCollectionType<Stdin>::value));
  EXPECT_EQ(CollectionType::STDOUT, (GetCollectionType<Stdout>::value));
  EXPECT_EQ(CollectionType::PERIODIC, (GetCollectionType<Periodic>::value));
}

}  // namespace fluent

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
