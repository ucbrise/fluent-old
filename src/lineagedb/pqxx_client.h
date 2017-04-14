#ifndef LINEAGEDB_PQXX_CLIENT_H_
#define LINEAGEDB_PQXX_CLIENT_H_

#include <cstdint>

#include "fmt/format.h"
#include "glog/logging.h"
#include "pqxx/pqxx"

#include "common/string_util.h"
#include "common/tuple_util.h"
#include "lineagedb/connection_config.h"

namespace fluent {
namespace lineagedb {
namespace detail {

inline std::int64_t size_t_to_int64(std::size_t hash) {
  return static_cast<std::int64_t>(hash);
}

// `ToSqlType<ToSql, T>()()` is equivalent to `ToSql<T>.Type()`.
template <template <typename> class ToSql, typename T>
struct ToSqlType {
  std::string operator()() { return ToSql<T>().Type(); }
};

}  // namespace detail

// # Overview
// Ignore the Connection and Work template arguments for now and pretend the
// InjectablePqxxClient class template really looks like this:
//
//   template <template <typename> class Hash, tempalte <typename> class ToSql>
//   class PqxxClient { ... }
//
// A `PqxxClient<Hash, ToSql>` object can be used to shuttle tuples and lineage
// information from a fluent node to a lineagedb database. It's best explained
// through an example:
//
//   // Construct a `ConnectionConfig` which tells our client to which lineagedb
//   // database it should connect and which credentials it should use to do
//   // so.
//   ConnectionConfig config {
//     "localhost", // host
//     5432,        // port
//     "username",  // username
//     "password",  // password
//     "sailors",   // the database
//   }
//
//   // Construct a client which will connect to the lineagedb database.
//   PqxxClient<Hash, ToSql> client(config);
//
//   // Initialize the lineagedb client with the name of our fluent node. A
//   // lineagedb client should be used by exactly one fluent node. Also, the
//   // name "lineage" is reserved; sorry about that.
//   client.Init("my_fluent_node");
//
//   // Add the types of our collections.
//   client.AddCollection<int, float>("t", "Table")   // Table t[int, float].
//   client.AddCollection<int, float>("c", "Channel") // Channel c[int, float].
//
//   // Add all our rules.
//   client.AddRule(0, true, t += c.Iterable());
//   client.AddRule(1, true, t -= (c.Iterable() | ra::filter(f)));
//   client.AddRule(0, false, t += c.Iterable());
//   client.AddRule(1, false, t -= (c.Iterable() | ra::filter(f)));
//
//   // Add and delete some tuples.
//   client.InsertTuple("t", 0 /* time_inserted */, make_tuple("hi",  42.0));
//   client.InsertTuple("t", 1 /* time_inserted */, make_tuple("bye", 14.0));
//   client.DeleteTuple("t", 2 /* time_deleted */,  make_tuple("bye", 14.0));
//
// Cool! But what about those Connection and Work template arguments? And why
// is it called InjectablePqxxClient? In short, InjectablePqxxClient is a
// dependency injected version of PqxxClient.
//
// The PqxxClient struct template we used above uses `pqxx::connection` and
// `pqxx::work` for `Connection` and `Work` to actually connect to a lineagedb
// database. If we don't want to actually connect to a database (say for unit
// tests), we can substitute a mock connection and work class in for
// `Connection` and `Work`. In fact, we do exactly that in
// `mock_pqxx_client.h`.
//
// # Implementation
// Refer to `README.md` in the `fluent` directory for a description of (and the
// schema of) the tables used to store a node's history and lineage. This class
// issues SQL queries to create and populate those tables.
template <typename Connection, typename Work, template <typename> class Hash,
          template <typename> class ToSql>
class InjectablePqxxClient {
 public:
  InjectablePqxxClient(std::string name, std::size_t id, std::string address,
                       const ConnectionConfig& connection_config)
      : connection_(connection_config.ToString()),
        name_(std::move(name)),
        id_(id),
        address_(std::move(address)) {
    LOG(INFO)
        << "Established a lineagedb connection with the following parameters: "
        << connection_config.ToString();
  }

  // TODO(mwhittaker): Handle hash collisions.
  void Init() {
    initialized_ = true;

    ExecuteQuery(
        "Init",
        fmt::format(R"(
      INSERT INTO Nodes (id, name, address)
      VALUES ({});
    )",
                    Join(SqlValues(std::make_tuple(id_, name_, address_)))));

    ExecuteQuery("CreateLineageTable", fmt::format(R"(
      CREATE TABLE {}_lineage (
        dep_node_id          bigint   NOT NULL,
        dep_collection_name  text     NOT NULL,
        dep_tuple_hash       bigint   NOT NULL,
        dep_time             bigint,
        rule_number          integer,
        inserted             boolean  NOT NULL,
        collection_name      text     NOT NULL,
        tuple_hash           bigint   NOT NULL,
        time                 integer  NOT NULL
      );
    )",
                                                   name_));
  }

  template <typename... Ts>
  void AddCollection(
      const std::string& collection_name, const std::string& collection_type,
      const std::array<std::string, sizeof...(Ts)>& column_names) {
    static_assert(sizeof...(Ts) > 0, "Collections should have >= 1 column.");
    CHECK(initialized_) << "Call Init first.";

    // For a fluent node `n` and collection `c`, we create a lineagedb relation
    // `n_c`. We also make a relation for the lineage of `n` called
    // `n_lineage`. Thus, we have a naming conflict if `c == lineage`.
    CHECK_NE(collection_name, std::string("lineage"))
        << "Lineage is a reserved collection name.";

    ExecuteQuery(
        "AddCollection",
        fmt::format(
            R"(
      INSERT INTO Collections (node_id, collection_name, collection_type,
                               column_names)
      VALUES ({});
    )",
            Join(SqlValues(std::make_tuple(id_, collection_name,
                                           collection_type, column_names)))));

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

  void AddRule(std::size_t rule_number, bool is_bootstrap,
               const std::string& rule_string) {
    CHECK(initialized_) << "Call Init first.";
    ExecuteQuery(
        "AddRule",
        fmt::format(R"(
      INSERT INTO Rules (node_id, rule_number, is_bootstrap, rule)
      VALUES ({});
    )",
                    Join(SqlValues(std::make_tuple(
                        id_, rule_number, is_bootstrap, rule_string)))));
  }

  template <typename... Ts>
  void InsertTuple(const std::string& collection_name, int time_inserted,
                   const std::tuple<Ts...>& t) {
    static_assert(sizeof...(Ts) > 0, "Collections should have >=1 column.");
    CHECK(initialized_) << "Call Init first.";
    std::int64_t hash = detail::size_t_to_int64(Hash<std::tuple<Ts...>>()(t));
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
    CHECK(initialized_) << "Call Init first.";
    std::int64_t hash = detail::size_t_to_int64(Hash<std::tuple<Ts...>>()(t));
    ExecuteQuery("DeleteTuple",
                 fmt::format(R"(
      UPDATE {}_{}
      SET time_deleted = {}
      WHERE hash={} AND time_deleted IS NULL;
    )",
                             name_, collection_name, time_deleted, hash));
  }

  void AddNetworkedLineage(std::size_t dep_node_id, int dep_time,
                           const std::string& collection_name,
                           std::size_t tuple_hash, int time) {
    CHECK(initialized_) << "Call Init first.";
    ExecuteQuery(
        "AddLineage",
        fmt::format(
            R"(
      INSERT INTO {}_lineage (dep_node_id, dep_collection_name, dep_tuple_hash,
                              dep_time, rule_number, inserted, collection_name,
                              tuple_hash, time)
      VALUES ({}, NULL, {});
    )",
            name_, Join(SqlValues(std::make_tuple(
                       detail::size_t_to_int64(dep_node_id), collection_name,
                       detail::size_t_to_int64(tuple_hash), dep_time))),
            Join(SqlValues(std::make_tuple(true /*inserted*/, collection_name,
                                           detail::size_t_to_int64(tuple_hash),
                                           time)))));
  }

  void AddDerivedLineage(const std::string& dep_collection_name,
                         std::size_t dep_tuple_hash, int rule_number,
                         bool inserted, const std::string& collection_name,
                         std::size_t tuple_hash, int time) {
    CHECK(initialized_) << "Call Init first.";
    ExecuteQuery(
        "AddLineage",
        fmt::format(
            R"(
      INSERT INTO {}_lineage (dep_node_id, dep_collection_name, dep_tuple_hash,
                              dep_time, rule_number, inserted, collection_name,
                              tuple_hash, time)
      VALUES ({}, NULL, {});
    )",
            name_, Join(SqlValues(std::make_tuple(
                       detail::size_t_to_int64(id_), dep_collection_name,
                       detail::size_t_to_int64(dep_tuple_hash)))),
            Join(SqlValues(
                std::make_tuple(rule_number, inserted, collection_name,
                                detail::size_t_to_int64(tuple_hash), time)))));
  }

 protected:
  // Transactionally execute the query `query` named `name`.
  virtual void ExecuteQuery(const std::string& name, const std::string& query) {
    Work txn(connection_, name);
    VLOG(1) << "Executing query: " << query;
    txn.exec(query);
    txn.commit();
  }

 private:
  // detail::ToSqlType partially applied to ToSql.
  template <typename T>
  using ToSqlType = detail::ToSqlType<ToSql, T>;

  // SqlTypes<T1, ... Tn> returns the vector
  // [ToSql<T1>.Type(), ..., ToSql<Tn>().Type()]
  template <typename... Ts>
  std::vector<std::string> SqlTypes() {
    return TupleToVector(TupleFromTypes<ToSqlType, Ts...>());
  }

  // SqlValues((x1: T1, ..., xn: Tn)) returns the vector
  // [ToSql<T1>().Value(x1), ..., ToSql<Tn>().Value(xn)].
  template <typename... Ts>
  std::vector<std::string> SqlValues(const std::tuple<Ts...>& t) {
    return TupleToVector(TupleMap(t, [](const auto& x) {
      return ToSql<typename std::decay<decltype(x)>::type>().Value(x);
    }));
  }

  // A connection to lineagedb database.
  Connection connection_;

  // True after `Init()` is called;
  bool initialized_ = false;

  // The name of fluent node using this client.
  const std::string name_;

  // Each fluent node named `n` has a unique id `hash(n)`.
  const std::int64_t id_;

  // The address of the fluent program.
  const std::string address_;
};

// See InjectablePqxxClient documentation above.
template <template <typename> class Hash, template <typename> class ToSql>
using PqxxClient =
    InjectablePqxxClient<pqxx::connection, pqxx::work, Hash, ToSql>;

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_PQXX_CLIENT_H_
