#ifndef LINEAGEDB_MOCK_CLIENT_H_
#define LINEAGEDB_MOCK_CLIENT_H_

#include <cstddef>

#include <array>
#include <chrono>
#include <memory>
#include <string>
#include <utility>

#include "common/macros.h"
#include "common/status.h"
#include "common/status_or.h"
#include "common/tuple_util.h"
#include "common/type_list.h"
#include "fluent/local_tuple_id.h"
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
template <template <typename> class Hash, template <typename> class ToSql,
          typename Clock>
class MockClient {
 public:
  DISALLOW_COPY_AND_ASSIGN(MockClient);
  DISALLOW_MOVE_AND_ASSIGN(MockClient);

  // Client Mocks //////////////////////////////////////////////////////////////
  static WARN_UNUSED common::StatusOr<std::unique_ptr<MockClient>> Make(
      std::string name, std::size_t id, std::string address,
      const ConnectionConfig& config) {
    return std::unique_ptr<MockClient>(
        new MockClient(std::move(name), id, std::move(address), config));
  }

  template <typename... Ts>
  WARN_UNUSED common::Status AddCollection(
      const std::string& collection_name, const std::string& collection_type,
      const std::array<std::string, sizeof...(Ts)>& column_names) {
    std::vector<std::string> columns;
    for (const std::string& column : column_names) {
      columns.push_back(column);
    }

    std::vector<std::string> types;
    common::TupleIter(
        common::TypeListMapToTuple<common::TypeList<Ts...>, ToSqlType>()(),
        [&types](const std::string& s) { types.push_back(s); });

    add_collection_.push_back(
        std::make_tuple(collection_name, collection_type, columns, types));

    return common::Status::OK;
  }

  WARN_UNUSED common::Status AddRule(std::size_t rule_number, bool is_bootstrap,
                                     const std::string& rule_string) {
    add_rule_.push_back(
        std::make_tuple(rule_number, is_bootstrap, rule_string));
    return common::Status::OK;
  }

  template <typename... Ts>
  WARN_UNUSED common::Status InsertTuple(
      const std::string& collection_name, int time_inserted,
      const std::chrono::time_point<Clock>& physical_time_inserted,
      const std::tuple<Ts...>& t) {
    auto strings_tuple = common::TupleMap(t, [](const auto& x) {
      return ToSql<typename std::decay<decltype(x)>::type>().Value(x);
    });
    std::vector<std::string> strings_vec;
    common::TupleIter(strings_tuple, [&strings_vec](const auto& s) {
      strings_vec.push_back(s);
    });
    insert_tuple_.push_back(std::make_tuple(
        collection_name, time_inserted, physical_time_inserted, strings_vec));
    return common::Status::OK;
  }

  template <typename... Ts>
  WARN_UNUSED common::Status DeleteTuple(
      const std::string& collection_name, int time_deleted,
      const std::chrono::time_point<Clock>& physical_time_deleted,
      const std::tuple<Ts...>& t) {
    auto strings_tuple = common::TupleMap(t, [](const auto& x) {
      return ToSql<typename std::decay<decltype(x)>::type>().Value(x);
    });
    std::vector<std::string> strings_vec;
    common::TupleIter(strings_tuple, [&strings_vec](const auto& s) {
      strings_vec.push_back(s);
    });
    delete_tuple_.push_back(std::make_tuple(
        collection_name, time_deleted, physical_time_deleted, strings_vec));
    return common::Status::OK;
  }

  WARN_UNUSED common::Status AddNetworkedLineage(
      std::size_t dep_node_id, int dep_time, const std::string& collection_name,
      std::size_t tuple_hash, int time) {
    add_networked_lineage_.push_back(std::make_tuple(
        dep_node_id, dep_time, collection_name, tuple_hash, time));
    return common::Status::OK;
  }

  WARN_UNUSED common::Status AddDerivedLineage(
      const LocalTupleId& dep_id, int rule_number, bool inserted,
      const std::chrono::time_point<Clock>& physical_time,
      const LocalTupleId& id) {
    add_derived_lineage_.push_back(
        std::make_tuple(dep_id, rule_number, inserted, physical_time, id));
    return common::Status::OK;
  }

  WARN_UNUSED common::Status RegisterBlackBoxLineage(
      const std::string& collection_name,
      const std::vector<std::string>& lineage_commands) {
    register_black_box_lineage_.push_back(
        std::make_tuple(collection_name, lineage_commands));
    return common::Status::OK;
  }

  WARN_UNUSED common::Status RegisterBlackBoxPythonLineageScript(
      const std::string& script) {
    register_black_box_python_lineage_script_.push_back(
        std::make_tuple(script));
    return common::Status::OK;
  }

  WARN_UNUSED common::Status RegisterBlackBoxPythonLineage(
      const std::string& collection_name, const std::string& method) {
    register_black_box_python_lineage_.push_back(
        std::make_tuple(collection_name, method));
    return common::Status::OK;
  }

  // Getters ///////////////////////////////////////////////////////////////////
  using AddCollectionTuple = std::tuple<  //
      std::string,                        // name
      std::string,                        // collection_type
      std::vector<std::string>,           // column names
      std::vector<std::string>>;          // column types

  using AddRuleTuple = std::tuple<  //
      std::size_t,                  // rule number
      bool,                         // is bootstrap rule
      std::string>;                 // rule string

  using InsertTupleTuple = std::tuple<  //
      std::string,                      // collection name
      int,                              // logical time inserted
      std::chrono::time_point<Clock>,   // physical time inserted
      std::vector<std::string>>;        // tuple

  using DeleteTupleTuple = std::tuple<  //
      std::string,                      // collection name
      int,                              // logical time deleted
      std::chrono::time_point<Clock>,   // physical time deleted
      std::vector<std::string>>;        // tuple

  using AddNetworkedLineageTuple = std::tuple<  //
      std::size_t,                              // dep_node_id
      int,                                      // dep_time
      std::string,                              // collection name
      std::size_t,                              // tuple hash
      int>;                                     // time

  using AddDerivedLineageTuple = std::tuple<  //
      LocalTupleId,                           // dep id
      int,                                    // rule number
      bool,                                   // inserted
      std::chrono::time_point<Clock>,         // physical time
      LocalTupleId>;                          // id

  using RegisterBlackBoxLineageTuple = std::tuple<  //
      std::string,                                  // collection name
      std::vector<std::string>>;                    // queries

  using RegisterBlackBoxPythonLineageScriptTuple = std::tuple<  //
      std::string>;                                             // script

  using RegisterBlackBoxPythonLineageTuple = std::tuple<  //
      std::string,                                        // collection name
      std::string>;                                       // method

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

  const std::vector<RegisterBlackBoxLineageTuple>& GetRegisterBlackBoxLineage()
      const {
    return register_black_box_lineage_;
  }

  const std::vector<RegisterBlackBoxPythonLineageScriptTuple>&
  GetRegisterBlackBoxPythonLineageScript() const {
    return register_black_box_python_lineage_script_;
  }

  const std::vector<RegisterBlackBoxPythonLineageTuple>&
  GetRegisterBlackBoxPythonLineage() const {
    return register_black_box_python_lineage_;
  }

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
  std::vector<RegisterBlackBoxLineageTuple> register_black_box_lineage_;
  std::vector<RegisterBlackBoxPythonLineageScriptTuple>
      register_black_box_python_lineage_script_;
  std::vector<RegisterBlackBoxPythonLineageTuple>
      register_black_box_python_lineage_;
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_MOCK_CLIENT_H_
