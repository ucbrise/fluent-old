#ifndef COLLECTIONS_COLLECTION_UTIL_H_
#define COLLECTIONS_COLLECTION_UTIL_H_

#include <chrono>
#include <string>

#include "collections/all.h"
#include "common/type_list.h"

namespace fluent {
namespace collections {

// `CollectionTypes` returns the types of the columns of a collection.
//
//   CollectionTypes<Table<Ts...>> == <Ts...>
//   CollectionTypes<Scratch<Ts...>> == <Ts...>
//   CollectionTypes<Channel<Pickler, Ts...>> == <Ts...>
//   CollectionTypes<Periodic<C>> == <Periodic<C>::id, time_point<C>>
//   CollectionTypes<Stdin> == <std::string>
//   CollectionTypes<Stdout> == <std::string>
template <typename Collection>
struct CollectionTypes;

template <typename... Ts>
struct CollectionTypes<Table<Ts...>> {
  using type = common::TypeList<Ts...>;
};

template <typename... Ts>
struct CollectionTypes<Scratch<Ts...>> {
  using type = common::TypeList<Ts...>;
};

template <template <typename> class Pickler, typename T, typename... Ts>
struct CollectionTypes<Channel<Pickler, T, Ts...>> {
  using type = common::TypeList<T, Ts...>;
};

template <>
struct CollectionTypes<Stdin> {
  using type = common::TypeList<std::string>;
};

template <>
struct CollectionTypes<Stdout> {
  using type = common::TypeList<std::string>;
};

template <typename Clock>
struct CollectionTypes<Periodic<Clock>> {
  using id = typename Periodic<Clock>::id;
  using time = std::chrono::time_point<Clock>;
  using type = common::TypeList<id, time>;
};

// Sometimes we want to be able to write code like this:
//
//   let is_templated collection =
//     match collection with
//     | Table<_>
//     | Scratch<_>
//     | Channel<_> -> true
//     | Stdin
//     | Stdout
//     | Periodic -> false
//
// but in order to do so, we end having to write code like this:
//
//   template <typename... Ts>
//   bool is_templated(const Table<Ts...>&) {
//     return true;
//   }
//
//   template <typename... Ts>
//   bool is_templated(const Scratch<Ts...>&) {
//     return true;
//   }
//
//   ...
//
// The CollectionType enum class provides a way to dispatch based on the type
// of a collection at runtime and helps avoid having to write a lot of template
// boilerplate. GetCollectionType can be used to get the CollectionType of a
// collection.
enum class CollectionType { TABLE, SCRATCH, CHANNEL, STDIN, STDOUT, PERIODIC };

template <typename Collection>
struct GetCollectionType;

template <typename... Ts>
struct GetCollectionType<Table<Ts...>>
    : public std::integral_constant<CollectionType, CollectionType::TABLE> {};

template <typename... Ts>
struct GetCollectionType<Scratch<Ts...>>
    : public std::integral_constant<CollectionType, CollectionType::SCRATCH> {};

template <template <typename> class Pickler, typename T, typename... Ts>
struct GetCollectionType<Channel<Pickler, T, Ts...>>
    : public std::integral_constant<CollectionType, CollectionType::CHANNEL> {};

template <>
struct GetCollectionType<Stdin>
    : public std::integral_constant<CollectionType, CollectionType::STDIN> {};

template <>
struct GetCollectionType<Stdout>
    : public std::integral_constant<CollectionType, CollectionType::STDOUT> {};

template <typename Clock>
struct GetCollectionType<Periodic<Clock>>
    : public std::integral_constant<CollectionType, CollectionType::PERIODIC> {
};

std::string CollectionTypeToString(CollectionType type);

}  // namespace collections
}  // namespace fluent

#endif  // COLLECTIONS_COLLECTION_UTIL_H_
