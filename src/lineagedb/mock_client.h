#ifndef LINEAGEDB_MOCK_CLIENT_H_
#define LINEAGEDB_MOCK_CLIENT_H_

#include <cstddef>

#include <array>
#include <string>
#include <utility>

#include "common/macros.h"
#include "common/status.h"
#include "common/status_or.h"
#include "common/tuple_util.h"
#include "common/type_list.h"
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
  MockClient() = default;
  DISALLOW_COPY_AND_ASSIGN(MockClient);
  MockClient(MockClient&&) = default;
  MockClient& operator=(MockClient&&) = default;

  // Client Mocks //////////////////////////////////////////////////////////////
  static WARN_UNUSED StatusOr<MockClient> Make(std::string name, std::size_t id,
                                               std::string address,
                                               const ConnectionConfig& config) {
    return MockClient(std::move(name), id, std::move(address), config);
  }

  template <typename... Ts>
  WARN_UNUSED Status AddCollection(
      const std::string& collection_name, const std::string& collection_type,
      const std::array<std::string, sizeof...(Ts)>& column_names) {
    std::vector<std::string> columns;
    for (const std::string& column : column_names) {
      columns.push_back(column);
    }

    std::vector<std::string> types;
    TupleIter(TypeListMapToTuple<TypeList<Ts...>, ToSqlType>()(),
              [&types](const std::string& s) { types.push_back(s); });

    add_collection_.push_back(
        std::make_tuple(collection_name, collection_type, columns, types));

    return Status::OK;
  }

  WARN_UNUSED Status AddRule(std::size_t rule_number, bool is_bootstrap,
                             const std::string& rule_string) {
    add_rule_.push_back(
        std::make_tuple(rule_number, is_bootstrap, rule_string));
    return Status::OK;
  }

  template <typename... Ts>
  WARN_UNUSED Status InsertTuple(const std::string& collection_name,
                                 int time_inserted,
                                 const std::tuple<Ts...>& t) {
    auto strings_tuple = TupleMap(t, [](const auto& x) {
      return ToSql<typename std::decay<decltype(x)>::type>().Value(x);
    });
    std::vector<std::string> strings_vec;
    TupleIter(strings_tuple,
              [&strings_vec](const auto& s) { strings_vec.push_back(s); });
    insert_tuple_.push_back(
        std::make_tuple(collection_name, time_inserted, strings_vec));
    return Status::OK;
  }

  template <typename... Ts>
  WARN_UNUSED Status DeleteTuple(const std::string& collection_name,
                                 int time_deleted, const std::tuple<Ts...>& t) {
    auto strings_tuple = TupleMap(t, [](const auto& x) {
      return ToSql<typename std::decay<decltype(x)>::type>().Value(x);
    });
    std::vector<std::string> strings_vec;
    TupleIter(strings_tuple,
              [&strings_vec](const auto& s) { strings_vec.push_back(s); });
    delete_tuple_.push_back(
        std::make_tuple(collection_name, time_deleted, strings_vec));
    return Status::OK;
  }

  WARN_UNUSED Status AddNetworkedLineage(std::size_t dep_node_id, int dep_time,
                                         const std::string& collection_name,
                                         std::size_t tuple_hash, int time) {
    add_networked_lineage_.push_back(std::make_tuple(
        dep_node_id, dep_time, collection_name, tuple_hash, time));
    return Status::OK;
  }

  WARN_UNUSED Status AddDerivedLineage(const std::string& dep_collection_name,
                                       std::size_t dep_tuple_hash,
                                       int rule_number, bool inserted,
                                       const std::string& collection_name,
                                       std::size_t tuple_hash, int time) {
    add_derived_lineage_.push_back(
        std::make_tuple(dep_collection_name, dep_tuple_hash, rule_number,
                        inserted, collection_name, tuple_hash, time));
    return Status::OK;
  }

  WARN_UNUSED Status Exec(const std::string& query) {
    exec_.push_back(query);
    return Status::OK;
  }

  // Getters ///////////////////////////////////////////////////////////////////
  // name, collection type, column names, column types
  using AddCollectionTuple =
      std::tuple<std::string, std::string, std::vector<std::string>,
                 std::vector<std::string>>;
  // rule number, is bootstrap rule, rule string
  using AddRuleTuple = std::tuple<std::size_t, bool, std::string>;
  // collection name, time inserted, tuple
  using InsertTupleTuple =
      std::tuple<std::string, int, std::vector<std::string>>;
  // collection name, time deleted, tuple
  using DeleteTupleTuple =
      std::tuple<std::string, int, std::vector<std::string>>;
  // dep_node_id, dep_time, collection_name, tuple_hash, time
  using AddNetworkedLineageTuple =
      std::tuple<std::size_t, int, std::string, std::size_t, int>;
  // dep_collection_name, dep_tuple_hash, rule_number, inserted,
  // collection_name, tuple_hash, time
  using AddDerivedLineageTuple = std::tuple<std::string, std::size_t, int, bool,
                                            std::string, std::size_t, int>;
  // query
  using ExecTuple = std::tuple<std::string>;

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
  const std::vector<ExecTuple>& GetExec() const { return exec_; }

 private:
  template <typename T>
  using ToSqlType = detail::ToSqlType<ToSql, T>;

  MockClient(std::string name, std::size_t id, std::string address,
             const ConnectionConfig& config)
      : name_(std::move(name)),
        id_(id),
        address_(std::move(address)),
        config_(config) {}

  const std::string name_;
  const std::size_t id_;
  const std::string address_;
  const ConnectionConfig config_;

  std::vector<AddCollectionTuple> add_collection_;
  std::vector<AddRuleTuple> add_rule_;
  std::vector<InsertTupleTuple> insert_tuple_;
  std::vector<DeleteTupleTuple> delete_tuple_;
  std::vector<AddNetworkedLineageTuple> add_networked_lineage_;
  std::vector<AddDerivedLineageTuple> add_derived_lineage_;
  std::vector<ExecTuple> exec_;
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_MOCK_CLIENT_H_
