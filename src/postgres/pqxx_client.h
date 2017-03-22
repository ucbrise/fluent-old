#ifndef POSTGRES_PQXX_CLIENT_H_
#define POSTGRES_PQXX_CLIENT_H_

#include <cstdint>

#include "fmt/format.h"
#include "glog/logging.h"
#include "pqxx/pqxx"

#include "common/string_util.h"
#include "common/tuple_util.h"
#include "postgres/connection_config.h"

namespace fluent {
namespace postgres {
namespace detail {

inline std::int64_t hash_to_sql(std::size_t hash) {
  return static_cast<std::int64_t>(hash);
}

template <template <typename> class ToSql, typename T>
struct ToSqlType {
  std::string operator()() { return ToSql<T>().Type(); }
};

}  // namespace detail

// DO_NOT_SUBMIT(mwhittaker): Document.
template <typename Connection, typename Work, template <typename> class Hash,
          template <typename> class ToSql>
class InjectablePqxxClient {
 public:
  InjectablePqxxClient(const ConnectionConfig& connection_config)
      : connection_(connection_config.ToString()), id_(0) {
    LOG(INFO)
        << "Established a postgres connection with the following parameters: "
        << connection_config.ToString();
  }

  void Init(const std::string& name) {
    // TODO(mwhittaker): Handle the scenario where there is a hash collision.
    // TODO(mwhittaker): Handle the scenario where the hash is 0.
    name_ = name;
    id_ = detail::hash_to_sql(Hash<std::string>()(name));

    ExecuteQuery("Init",
                 fmt::format(R"(
      INSERT INTO Nodes (id, name)
      VALUES ({});
    )",
                             Join(SqlValues(std::make_tuple(id_, name)))));

    ExecuteQuery("CreateLineageTable", fmt::format(R"(
      CREATE TABLE {}_lineage (
        dep_node_id          bigint   NOT NULL,
        dep_collection_name  text     NOT NULL,
        dep_tuple_hash       bigint   NOT NULL,
        rule_number          integer  NOT NULL,
        inserted             boolean  NOT NULL,
        collection_name      text     NOT NULL,
        tuple_hash           bigint   NOT NULL,
        time                 integer  NOT NULL
      );
    )",
                                                   name));
  }

  template <typename... Ts>
  void AddCollection(const std::string& collection_name) {
    static_assert(sizeof...(Ts) > 0, "Collections should have >=1 column.");
    CHECK_NE(id_, static_cast<std::int64_t>(0)) << "Call Init first.";
    CHECK_NE(collection_name, std::string("lineage"))
        << "Lineage is a reserved collection name.";

    ExecuteQuery(
        "AddCollection",
        fmt::format(R"(
      INSERT INTO Collections (node_id, collection_name)
      VALUES ({});
    )",
                    Join(SqlValues(std::make_tuple(id_, collection_name)))));

    std::vector<std::string> types = SqlTypes<Ts...>();
    std::vector<std::string> columns;
    for (std::size_t i = 0; i < types.size(); ++i) {
      columns.push_back(fmt::format("col_{} {} NOT NULL", i, types[i]));
    }
    ExecuteQuery("AddCollectionTable",
                 fmt::format(R"(
      CREATE TABLE {}_{} (
        hash          bigint  NOT NULL,
        time_inserted integer NOT NULL,
        time_deleted  integer,
        {},
        PRIMARY KEY (hash, time_inserted)
      );
    )",
                             name_, collection_name, Join(columns)));
  }

  template <typename RA>
  void AddRule(std::size_t rule_number, const RA& rule) {
    CHECK_NE(id_, static_cast<std::int64_t>(0)) << "Call Init first.";
    auto rule_string = std::get<2>(rule).ToDebugString();
    ExecuteQuery("AddRule", fmt::format(R"(
      INSERT INTO Rules (node_id, rule_number, rule)
      VALUES ({});
    )",
                                        Join(SqlValues(std::make_tuple(
                                            id_, rule_number, rule_string)))));
  }

  template <typename... Ts>
  void InsertTuple(const std::string& collection_name, int time_inserted,
                   const std::tuple<Ts...>& t) {
    static_assert(sizeof...(Ts) > 0, "Collections should have >=1 column.");
    std::int64_t hash = detail::hash_to_sql(Hash<std::tuple<Ts...>>()(t));
    ExecuteQuery("InsertTuple", fmt::format(R"(
      INSERT INTO {}_{}
      VALUES ({}, {}, NULL, {});
    )",
                                            name_, collection_name, hash,
                                            time_inserted, Join(SqlValues(t))));
  }

  template <typename... Ts>
  void DeleteTuple(const std::string& collection_name, int time_deleted,
                   const std::tuple<Ts...>& t) {
    static_assert(sizeof...(Ts) > 0, "Collections should have >=1 column.");
    std::int64_t hash = detail::hash_to_sql(Hash<std::tuple<Ts...>>()(t));
    ExecuteQuery("DeleteTuple",
                 fmt::format(R"(
      UPDATE {}_{}
      SET time_deleted = {}
      WHERE hash={} AND time_deleted IS NULL;
    )",
                             name_, collection_name, time_deleted, hash));
  }

 protected:
  virtual void ExecuteQuery(const std::string& name, const std::string& query) {
    Work txn(connection_, name);
    VLOG(1) << "Executing query: " << query;
    txn.exec(query);
    txn.commit();
  }

 private:
  template <typename T>
  using ToSqlType = detail::ToSqlType<ToSql, T>;

  template <typename... Ts>
  std::vector<std::string> SqlTypes() {
    return TupleToVector(TupleFromTypes<ToSqlType, Ts...>());
  }

  template <typename... Ts>
  std::vector<std::string> SqlValues(const std::tuple<Ts...>& t) {
    return TupleToVector(TupleMap(t, [](const auto& x) {
      return ToSql<typename std::decay<decltype(x)>::type>().Value(x);
    }));
  }

  Connection connection_;
  std::int64_t id_;
  std::string name_;
};

template <template <typename> class Hash, template <typename> class ToSql>
using PqxxClient =
    InjectablePqxxClient<pqxx::connection, pqxx::work, Hash, ToSql>;

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_PQXX_CLIENT_H_
