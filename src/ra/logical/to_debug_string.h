#ifndef RA_LOGICAL_TO_DEBUG_STRING_H_
#define RA_LOGICAL_TO_DEBUG_STRING_H_

#include <cstddef>

#include <string>
#include <type_traits>

#include "fmt/format.h"

#include "common/static_assert.h"
#include "common/string_util.h"
#include "ra/logical/all.h"

namespace fluent {
namespace ra {
namespace logical {

template <typename Ra, typename RaDecayed = typename std::decay<Ra>::type>
std::string ToDebugString(const Ra& ra);

template <typename LogicalRa>
struct ToDebugStringImpl;

template <typename C>
struct ToDebugStringImpl<Collection<C>> {
  std::string operator()(const Collection<C>& ra) {
    return fmt::format("Collection({})", ra.collection->Name());
  }
};

template <typename C>
struct ToDebugStringImpl<MetaCollection<C>> {
  std::string operator()(const MetaCollection<C>& ra) {
    return fmt::format("MetaCollection({})", ra.collection->Name());
  }
};

template <typename C>
struct ToDebugStringImpl<Iterable<C>> {
  std::string operator()(const Iterable<C>&) { return "Iterable"; }
};

template <typename Ra, typename F>
struct ToDebugStringImpl<Map<Ra, F>> {
  std::string operator()(const Map<Ra, F>& map) {
    return fmt::format("Map({})", ToDebugString(map.child));
  }
};

template <typename Ra, typename F>
struct ToDebugStringImpl<Filter<Ra, F>> {
  std::string operator()(const Filter<Ra, F>& filter) {
    return fmt::format("Filter({})", ToDebugString(filter.child));
  }
};

template <typename Ra, std::size_t... Is>
struct ToDebugStringImpl<Project<Ra, Is...>> {
  std::string operator()(const Project<Ra, Is...>& project) {
    const std::string columns = Join(Is...);
    const std::string child = ToDebugString(project.child);
    return fmt::format("Project<{}>({})", columns, child);
  }
};

template <typename Left, typename Right>
struct ToDebugStringImpl<Cross<Left, Right>> {
  std::string operator()(const Cross<Left, Right>& cross) {
    const std::string left = ToDebugString(cross.left);
    const std::string right = ToDebugString(cross.right);
    return fmt::format("Cross({}, {})", left, right);
  }
};

template <typename Left, std::size_t... LeftKs,  //
          typename Right, std::size_t... RightKs>
struct ToDebugStringImpl<HashJoin<Left, LeftKeys<LeftKs...>,  //
                                  Right, RightKeys<RightKs...>>> {
  std::string operator()(
      const HashJoin<Left, LeftKeys<LeftKs...>,  //
                     Right, RightKeys<RightKs...>>& hash_join) {
    const std::string left = ToDebugString(hash_join.left);
    const std::string left_keys = Join(LeftKs...);
    const std::string right = ToDebugString(hash_join.right);
    const std::string right_keys = Join(RightKs...);
    return fmt::format("HashJoin<LeftKeys<{}>, RightKeys<{}>>({}, {})",  //
                       left_keys, right_keys, left, right);
  }
};

template <typename Ra, std::size_t... Ks, typename... Aggregates>
struct ToDebugStringImpl<GroupBy<Ra, Keys<Ks...>, Aggregates...>> {
  std::string operator()(
      const GroupBy<Ra, Keys<Ks...>, Aggregates...>& group_by) {
    const std::string columns = Join(Ks...);
    const std::string aggregates = Join(Aggregates::ToDebugString()...);
    const std::string child = ToDebugString(group_by.child);
    return fmt::format("GroupBy<Keys<{}>, {}>({})", columns, aggregates, child);
  }
};

template <typename Ra, typename RaDecayed>
std::string ToDebugString(const Ra& ra) {
  static_assert(StaticAssert<std::is_base_of<LogicalRa, RaDecayed>>::value, "");
  return ToDebugStringImpl<RaDecayed>()(ra);
}

}  // namespace logical
}  // namespace ra
}  // namespace fluent

#endif  // RA_LOGICAL_TO_DEBUG_STRING_H_
