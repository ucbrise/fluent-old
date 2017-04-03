#ifndef POSTGRES_MOCK_CLIENT_H_
#define POSTGRES_MOCK_CLIENT_H_

#include <cstddef>

#include <utility>

#include "fmt/format.h"

#include "common/tuple_util.h"
#include "postgres/connection_config.h"

namespace fluent {
namespace postgres {
namespace detail {

template <template <typename> class ToSql, typename T>
struct ToSqlType {
  std::string operator()() { return ToSql<T>().Type(); }
};

}  // namespace detail

// DO_NOT_SUBMIT(mwhittaker): Document.
template <template <typename> class Hash, template <typename> class ToSql>
class MockClient {
 public:
  // Client Mocks //////////////////////////////////////////////////////////////
  MockClient(std::string name, std::size_t id, const ConnectionConfig& config)
      : name_(std::move(name)), id_(id), config_(config) {}

  void Init() { init_ = true; }

  template <typename... Ts>
  void AddCollection(const std::string& collection_name) {
    std::vector<std::string> types;
    TupleIter(TupleFromTypes<ToSqlType, Ts...>(),
              [&types](const std::string& s) { types.push_back(s); });
    add_collection_.push_back(std::make_pair(collection_name, types));
  }

  template <typename Rule>
  void AddRule(std::size_t rule_number, const Rule& rule) {
    add_rule_.push_back(std::make_pair(
        rule_number, fmt::format("{} {} {}", std::get<0>(rule)->Name(),
                                 std::get<1>(rule).ToDebugString(),
                                 std::get<2>(rule).ToDebugString())));
  }

  template <typename... Ts>
  void InsertTuple(const std::string& collection_name, int time_inserted,
                   const std::tuple<Ts...>& t) {
    auto strings_tuple = TupleMap(t, [](const auto& x) {
      return ToSql<typename std::decay<decltype(x)>::type>().Value(x);
    });
    std::vector<std::string> strings_vec;
    TupleIter(strings_tuple,
              [&strings_vec](const auto& s) { strings_vec.push_back(s); });
    insert_tuple_.push_back(
        std::make_tuple(collection_name, time_inserted, strings_vec));
  }

  template <typename... Ts>
  void DeleteTuple(const std::string& collection_name, int time_deleted,
                   const std::tuple<Ts...>& t) {
    auto strings_tuple = TupleMap(t, [](const auto& x) {
      return ToSql<typename std::decay<decltype(x)>::type>().Value(x);
    });
    std::vector<std::string> strings_vec;
    TupleIter(strings_tuple,
              [&strings_vec](const auto& s) { strings_vec.push_back(s); });
    delete_tuple_.push_back(
        std::make_tuple(collection_name, time_deleted, strings_vec));
  }

  void AddNetworkedLineage(std::size_t dep_node_id, int dep_time,
                           const std::string& collection_name,
                           std::size_t tuple_hash, int time) {
    add_networked_lineage_.push_back(std::make_tuple(
        dep_node_id, dep_time, collection_name, tuple_hash, time));
  }

  void AddDerivedLineage(const std::string& dep_collection_name,
                         std::size_t dep_tuple_hash, int rule_number,
                         bool inserted, const std::string& collection_name,
                         std::size_t tuple_hash, int time) {
    add_derived_lineage_.push_back(
        std::make_tuple(dep_collection_name, dep_tuple_hash, rule_number,
                        inserted, collection_name, tuple_hash, time));
  }

  // Getters ///////////////////////////////////////////////////////////////////
  bool GetInit() { return init_; }
  const auto& GetAddCollection() { return add_collection_; }
  const auto& GetAddRule() { return add_rule_; }
  const auto& GetInsertTuple() { return insert_tuple_; }
  const auto& GetDeleteTuple() { return delete_tuple_; }
  const auto& GetAddNetworkedLineage() { return add_networked_lineage_; }
  const auto& GetAddDerivedLineage() { return add_derived_lineage_; }

 private:
  template <typename T>
  using ToSqlType = detail::ToSqlType<ToSql, T>;

  const std::string name_;
  const std::size_t id_;
  const ConnectionConfig config_;

  // True iff Init has been called.
  bool init_ = false;

  // Every time `AddCollection<Ts...>(name)` is called, the pair `(name,
  // (ToSql<Ts>().Type()...))` is appended to `add_collection_`.
  std::vector<std::pair<std::string, std::vector<std::string>>> add_collection_;

  // Every time `AddRule(i, rule)` is called, the pair `(i, stringified rule)`
  // is appended to `add_rule_`.
  std::vector<std::pair<std::size_t, std::string>> add_rule_;

  // Every time `InsertTuple(name, time, (t1, ..., tn))` is called, the tuple
  // `(name, time, [ToSql<decltype(ti)>()(ti)...])` is appended to
  // `insert_tuple_`.
  std::vector<std::tuple<std::string, int, std::vector<std::string>>>
      insert_tuple_;

  // Every time `DeleteTuple(name, time, (t1, ..., tn))` is called, the tuple
  // `(name, time, [ToSql<decltype(ti)>()(ti)...])` is appended to
  // `delete_tuple_`.
  std::vector<std::tuple<std::string, int, std::vector<std::string>>>
      delete_tuple_;

  // Every time `AddNetworkedLineage(dn, dtime, name, hash, time)` is called,
  // the tuple `(dn, dtime, name, hash, time)` is appended to
  // `add_networked_lineage_`.
  std::vector<std::tuple<std::size_t, int, std::string, std::size_t, int>>
      add_networked_lineage_;

  // Every time `AddDerivedLineage(dn, dh, rn, i, n, h, t)` is called, the
  // tuple `(dn, dh, rn, i, n, h, t)` is appended to `add_derived_lineage_`.
  std::vector<std::tuple<std::string, std::size_t, int, bool, std::string,
                         std::size_t, int>>
      add_derived_lineage_;
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_MOCK_CLIENT_H_
