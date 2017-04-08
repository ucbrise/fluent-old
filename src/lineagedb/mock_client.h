#ifndef LINEAGEDB_MOCK_CLIENT_H_
#define LINEAGEDB_MOCK_CLIENT_H_

#include <cstddef>

#include <utility>

#include "fmt/format.h"

#include "common/tuple_util.h"
#include "lineagedb/connection_config.h"

namespace fluent {
namespace lineagedb {
namespace detail {

template <template <typename> class ToSql, typename T>
struct ToSqlType {
  std::string operator()() { return ToSql<T>().Type(); }
};

}  // namespace detail

// A MockClient has the same interface as a PqxxClient, but instead of
// constructing SQL queries and issuing them to a postgres database, a
// MockClient simply records invocations of its methods. MockClient is
// primarily for testing.
template <template <typename> class Hash, template <typename> class ToSql>
class MockClient {
 public:
  // Client Mocks //////////////////////////////////////////////////////////////
  MockClient(std::string name, std::size_t id, std::string address,
             const ConnectionConfig& config)
      : name_(std::move(name)),
        id_(id),
        address_(std::move(address)),
        config_(config) {}

  void Init() { init_ = true; }

  template <typename... Ts>
  void AddCollection(const std::string& collection_name) {
    std::vector<std::string> types;
    TupleIter(TupleFromTypes<ToSqlType, Ts...>(),
              [&types](const std::string& s) { types.push_back(s); });
    add_collection_.push_back(std::make_pair(collection_name, types));
  }

  void AddRule(std::size_t rule_number, bool is_bootstrap,
               const std::string& rule_string) {
    add_rule_.push_back(
        std::make_tuple(rule_number, is_bootstrap, rule_string));
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
  using AddCollectionTuple = std::tuple<std::string, std::vector<std::string>>;
  using AddRuleTuple = std::tuple<std::size_t, bool, std::string>;
  using InsertTupleTuple =
      std::tuple<std::string, int, std::vector<std::string>>;
  using DeleteTupleTuple =
      std::tuple<std::string, int, std::vector<std::string>>;
  using AddNetworkedLineageTuple =
      std::tuple<std::size_t, int, std::string, std::size_t, int>;
  using AddDerivedLineageTuple = std::tuple<std::string, std::size_t, int, bool,
                                            std::string, std::size_t, int>;

  bool GetInit() const { return init_; }
  const std::vector<AddCollectionTuple>& GetAddCollection() const {
    return add_collection_;
  }
  const std::vector<AddRuleTuple>& GetAddRule() const { return add_rule_; }
  const std::vector<InsertTupleTuple>& GetInsertTuple() const {
    return insert_tuple_;
  }
  const std::vector<DeleteTupleTuple>& GetDeleteTuple() const {
    return delete_tuple_;
  }
  const std::vector<AddNetworkedLineageTuple>& GetAddNetworkedLineage() const {
    return add_networked_lineage_;
  }
  const std::vector<AddDerivedLineageTuple>& GetAddDerivedLineage() const {
    return add_derived_lineage_;
  }

 private:
  template <typename T>
  using ToSqlType = detail::ToSqlType<ToSql, T>;

  const std::string name_;
  const std::size_t id_;
  const std::string address_;
  const ConnectionConfig config_;

  // True iff Init has been called.
  bool init_ = false;

  // Every time `AddCollection<Ts...>(name)` is called, the pair `(name,
  // (ToSql<Ts>().Type()...))` is appended to `add_collection_`.
  std::vector<AddCollectionTuple> add_collection_;

  // Every time `AddRule(i, b, rule)` is called, the pair `(i, b, rule rule)`
  // is appended to `add_rule_`.
  std::vector<AddRuleTuple> add_rule_;

  // Every time `InsertTuple(name, time, (t1, ..., tn))` is called, the tuple
  // `(name, time, [ToSql<decltype(ti)>()(ti)...])` is appended to
  // `insert_tuple_`.
  std::vector<InsertTupleTuple> insert_tuple_;

  // Every time `DeleteTuple(name, time, (t1, ..., tn))` is called, the tuple
  // `(name, time, [ToSql<decltype(ti)>()(ti)...])` is appended to
  // `delete_tuple_`.
  std::vector<DeleteTupleTuple> delete_tuple_;

  // Every time `AddNetworkedLineage(dn, dtime, name, hash, time)` is called,
  // the tuple `(dn, dtime, name, hash, time)` is appended to
  // `add_networked_lineage_`.
  std::vector<AddNetworkedLineageTuple> add_networked_lineage_;

  // Every time `AddDerivedLineage(dn, dh, rn, i, n, h, t)` is called, the
  // tuple `(dn, dh, rn, i, n, h, t)` is appended to `add_derived_lineage_`.
  std::vector<AddDerivedLineageTuple> add_derived_lineage_;
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_MOCK_CLIENT_H_
