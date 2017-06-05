#ifndef FLUENT_COLLECTION_UTIL_H_
#define FLUENT_COLLECTION_UTIL_H_

#include <string>
#include "common/type_list.h"
#include "fluent/channel.h"
#include "fluent/periodic.h"
#include "fluent/scratch.h"
#include "fluent/stdin.h"
#include "fluent/stdout.h"
#include "fluent/table.h"

namespace fluent {

// `CollectionTypes` returns the types of the columns of a collection.
//
//   - CollectionTypes<Table<Ts...>>   == TypeList<Ts...>
//   - CollectionTypes<Scratch<Ts...>> == TypeList<Ts...>
//   - CollectionTypes<Channel<Ts...>> == TypeList<Ts...>
//   - CollectionTypes<Stdin>          == TypeList<std::string>
//   - CollectionTypes<Stdout>         == TypeList<std::string>
//   - CollectionTypes<Periodic>       == TypeList<Periodic::id, Periodic::time>
template <typename Collection>
struct CollectionTypes;

template <typename... Ts>
struct CollectionTypes<Table<Ts...>> {
  using type = TypeList<Ts...>;
};

template <typename... Ts>
struct CollectionTypes<Scratch<Ts...>> {
  using type = TypeList<Ts...>;
};

template <typename... Ts>
struct CollectionTypes<Channel<Ts...>> {
  using type = TypeList<Ts...>;
};

template <>
struct CollectionTypes<Stdin> {
  using type = TypeList<std::string>;
};

template <>
struct CollectionTypes<Stdout> {
  using type = TypeList<std::string>;
};

template <>
struct CollectionTypes<Periodic> {
  using type = TypeList<Periodic::id, Periodic::time>;
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
//   template <typename... Ts>
//   bool is_templated(const Channel<Ts...>&) {
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

template <typename... Ts>
struct GetCollectionType<Channel<Ts...>>
    : public std::integral_constant<CollectionType, CollectionType::CHANNEL> {};

template <>
struct GetCollectionType<Stdin>
    : public std::integral_constant<CollectionType, CollectionType::STDIN> {};

template <>
struct GetCollectionType<Stdout>
    : public std::integral_constant<CollectionType, CollectionType::STDOUT> {};

template <>
struct GetCollectionType<Periodic>
    : public std::integral_constant<CollectionType, CollectionType::PERIODIC> {
};

std::string CollectionTypeToString(CollectionType type);

}  // namespace fluent

#endif  // FLUENT_COLLECTION_UTIL_H_
