#ifndef LINEAGEDB_PQXX_CLIENT_H_
#define LINEAGEDB_PQXX_CLIENT_H_

#include <cstdint>

#include <chrono>
#include <memory>

#include "fmt/format.h"
#include "glog/logging.h"
#include "pqxx/pqxx"

#include "common/macros.h"
#include "common/status.h"
#include "common/status_macros.h"
#include "common/status_or.h"
#include "common/string_util.h"
#include "common/tuple_util.h"
#include "common/type_list.h"
#include "fluent/local_tuple_id.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/to_sql.h"

namespace fluent {
namespace lineagedb {

// # Overview
// Ignore the Connection and Work template arguments for now and pretend the
// InjectablePqxxClient class template really looks like this:
//
//   template <
//    template <typename> class Hash,
//    template <typename> class ToSql,
//    typename Clock
//   >
//   class PqxxClient { ... }
//
// A `PqxxClient<Hash, ToSql, Clock>` object can be used to shuttle tuples and
// lineage information from a fluent node to a lineagedb database. It's best
// explained through an example:
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
//   using Client = PqxxClient<Hash, ToSql, system_clock>;
//   std::string name = "seanconnery";
//   std::size_t id = 42;
//   std::string address = "inproc://zardoz";
//   StatusOr<Client> client_or = Client::Make(name, id, address, config);
//   Client client = client_or.ConsumeValueOrDie();
//
//   // Add the types of our collections:
//   //   1. Table t[int, float].
//   //   2. Channel c[int, float].
//   client.AddCollection<int, float>("t", "Table", {{"x", "y"}})
//   client.AddCollection<int, float>("c", "Channel", {{"x", "y"}})
//
//   // Add all our rules.
//   client.AddRule(0, true, "t += c.Iterable()");
//   client.AddRule(1, true, "t -= (c.Iterable() | ra::filter(f))");
//   client.AddRule(0, false, "t += c.Iterable()");
//   client.AddRule(1, false, "t -= (c.Iterable() | ra::filter(f))");
//
//   // Add and delete some tuples.
//   client.InsertTuple("t", 0, system_clock::now(), make_tuple("hi",  42.0));
//   client.InsertTuple("t", 1, system_clock::now(), make_tuple("bye", 14.0));
//   client.DeleteTuple("t", 2, system_clock::now(), make_tuple("bye", 14.0));
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
//
// TODO(mwhittaker): Document these functions better.
template <typename Connection, typename Work, template <typename> class Hash,
          template <typename> class ToSql, typename Clock>
class InjectablePqxxClient {
 public:
  using time_point = std::chrono::time_point<Clock>;

  DISALLOW_COPY_AND_ASSIGN(InjectablePqxxClient);
  DISALLOW_MOVE_AND_ASSIGN(InjectablePqxxClient);
  virtual ~InjectablePqxxClient() = default;

  static WARN_UNUSED common::StatusOr<std::unique_ptr<InjectablePqxxClient>>
  Make(std::string name, std::size_t id, std::string address,
       const ConnectionConfig& connection_config) {
    try {
      std::unique_ptr<InjectablePqxxClient> client(new InjectablePqxxClient(
          std::move(name), id, std::move(address), connection_config));
      RETURN_IF_ERROR(client->Init());
      return std::move(client);
    } catch (const pqxx::pqxx_exception& e) {
      return common::Status(common::ErrorCode::INVALID_ARGUMENT,
                            e.base().what());
    }
  }

  template <typename... Ts>
  WARN_UNUSED common::Status AddCollection(
      const std::string& collection_name, const std::string& collection_type,
      const std::array<std::string, sizeof...(Ts)>& column_names) {
    static_assert(sizeof...(Ts) > 0, "Collections should have >= 1 column.");

    // For a fluent node `n` and collection `c`, we create a lineagedb relation
    // `n_c`. We also make a relation for the lineage of `n` called
    // `n_lineage`. Thus, we have a naming conflict if `c == lineage`.
    if (collection_name == "lineage") {
      return common::Status(common::ErrorCode::INVALID_ARGUMENT,
                            "Lineage is a reserved collection name.");
    }

    const std::string q1 = R"(
      INSERT INTO Collections (node_id,
                               collection_name,
                               collection_type,
                               column_names,
                               lineage_type,
                               python_lineage_method)
      VALUES ({id}, {collection}, {type}, {cols}, 'regular', NULL);
    )";
    RETURN_IF_ERROR(ExecuteQuery(
        "AddCollection",
        fmt::format(q1,  //
                    fmt::arg("id", SqlValue(id_)),
                    fmt::arg("collection", SqlValue(collection_name)),
                    fmt::arg("type", SqlValue(collection_type)),
                    fmt::arg("cols", SqlValue(column_names)))));

    std::vector<std::string> types = SqlTypes<Ts...>();
    std::vector<std::string> columns;
    for (std::size_t i = 0; i < types.size(); ++i) {
      columns.push_back(
          fmt::format("{} {} NOT NULL", column_names[i], types[i]));
    }
    const std::string q2 = R"(
      CREATE TABLE {name}_{collection} (
        hash                   {sizet_type} NOT NULL,
        time_inserted          {int_type}   NOT NULL,
        time_deleted           {int_type},
        physical_time_inserted {time_type}  NOT NULL,
        physical_time_deleted  {time_type},
        {columns},
        PRIMARY KEY (hash, time_inserted)
      );
    )";
    return ExecuteQuery(
        "AddCollectionTable",
        fmt::format(q2,  //
                    fmt::arg("name", name_),
                    fmt::arg("collection", collection_name),
                    fmt::arg("sizet_type", ToSql<std::size_t>().Type()),
                    fmt::arg("int_type", ToSql<int>().Type()),
                    fmt::arg("time_type", ToSql<time_point>().Type()),
                    fmt::arg("columns", common::Join(columns))));
  }

  WARN_UNUSED common::Status AddRule(std::size_t rule_number, bool is_bootstrap,
                                     const std::string& rule_string) {
    const std::string q = R"(
      INSERT INTO Rules (node_id, rule_number, is_bootstrap, rule)
      VALUES ({id}, {num}, {bootstrap}, {rule});
    )";
    return ExecuteQuery(
        "AddRule", fmt::format(q,  //
                               fmt::arg("id", SqlValue(id_)),
                               fmt::arg("num", SqlValue(rule_number)),
                               fmt::arg("bootstrap", SqlValue(is_bootstrap)),
                               fmt::arg("rule", SqlValue(rule_string))));
  }

  template <typename... Ts>
  WARN_UNUSED common::Status InsertTuple(
      const std::string& collection_name, int time_inserted,
      const time_point& physical_time_inserted, const std::tuple<Ts...>& t) {
    static_assert(sizeof...(Ts) > 0, "Collections should have >=1 column.");
    std::size_t hash = Hash<std::tuple<Ts...>>()(t);
    const std::string q = R"(
      INSERT INTO {name}_{collection}
      VALUES ({hash}, {ltime}, NULL, {ptime}, NULL, {t});
    )";
    return ExecuteQuery(
        "InsertTuple",
        fmt::format(q,  //
                    fmt::arg("name", name_),
                    fmt::arg("collection", collection_name),
                    fmt::arg("hash", SqlValue(hash)),
                    fmt::arg("ltime", SqlValue(time_inserted)),
                    fmt::arg("ptime", SqlValue(physical_time_inserted)),
                    fmt::arg("t", common::Join(SqlValues(t)))));
  }

  template <typename... Ts>
  WARN_UNUSED common::Status DeleteTuple(
      const std::string& collection_name, int time_deleted,
      const time_point& physical_time_deleted, const std::tuple<Ts...>& t) {
    static_assert(sizeof...(Ts) > 0, "Collections should have >=1 column.");
    std::size_t hash = Hash<std::tuple<Ts...>>()(t);
    const std::string q = R"(
      UPDATE {name}_{collection}
      SET time_deleted = {ltime}, physical_time_deleted = {ptime}
      WHERE hash = {hash} AND time_deleted IS NULL;
    )";
    return ExecuteQuery(
        "DeleteTuple",
        fmt::format(q,  //
                    fmt::arg("name", name_),
                    fmt::arg("collection", collection_name),
                    fmt::arg("ltime", SqlValue(time_deleted)),
                    fmt::arg("ptime", SqlValue(physical_time_deleted)),
                    fmt::arg("hash", SqlValue(hash))));
  }

  // TODO(mwhittaker): Add physical time to network lineage.
  WARN_UNUSED common::Status AddNetworkedLineage(
      std::size_t dep_node_id, int dep_time, const std::string& collection_name,
      std::size_t tuple_hash, int time) {
    const std::string q = R"(
      INSERT INTO {name}_lineage (dep_node_id, dep_collection_name,
                                  dep_tuple_hash, dep_time, rule_number,
                                  inserted, collection_name, tuple_hash, time)
      VALUES ({dep_id}, {dep_c}, {dep_hash}, {dep_time},
              NULL,
              {inserted}, {c}, {hash}, {time});
    )";
    return ExecuteQuery(
        "AddLineage",
        fmt::format(q,  //
                    fmt::arg("name", name_),
                    fmt::arg("dep_id", SqlValue(dep_node_id)),
                    fmt::arg("dep_c", SqlValue(collection_name)),
                    fmt::arg("dep_hash", SqlValue(tuple_hash)),
                    fmt::arg("dep_time", SqlValue(dep_time)),
                    fmt::arg("inserted", SqlValue(true)),
                    fmt::arg("c", SqlValue(collection_name)),
                    fmt::arg("hash", SqlValue(tuple_hash)),
                    fmt::arg("time", SqlValue(time))));
  }

  WARN_UNUSED common::Status AddDerivedLineage(const LocalTupleId& dep_id,
                                               int rule_number, bool inserted,
                                               const time_point& physical_time,
                                               const LocalTupleId& id) {
    const std::string q = R"(
      INSERT INTO {name}_lineage (dep_node_id, dep_collection_name,
                                  dep_tuple_hash, dep_time, rule_number,
                                  inserted, physical_time, collection_name,
                                  tuple_hash, time)
      VALUES ({dep_id}, {dep_c}, {dep_hash}, {dep_time},
              {rule}, {inserted}, {ptime},
              {c}, {hash}, {time});
    )";
    return ExecuteQuery(
        "AddLineage",
        fmt::format(
            q,                        //
            fmt::arg("name", name_),  //
            fmt::arg("dep_id", SqlValue(id_)),
            fmt::arg("dep_c", SqlValue(dep_id.collection_name)),
            fmt::arg("dep_hash", SqlValue(dep_id.hash)),
            fmt::arg("dep_time", SqlValue(dep_id.logical_time_inserted)),
            fmt::arg("rule", SqlValue(rule_number)),
            fmt::arg("inserted", SqlValue(inserted)),
            fmt::arg("ptime", SqlValue(physical_time)),
            fmt::arg("c", SqlValue(id.collection_name)),
            fmt::arg("hash", SqlValue(id.hash)),
            fmt::arg("time", SqlValue(id.logical_time_inserted))));
  }

  WARN_UNUSED common::Status RegisterBlackBoxLineage(
      const std::string& collection_name,
      const std::vector<std::string>& lineage_commands) {
    const std::string query_template = R"(
      UPDATE Collections
      SET lineage_type = 'sql'
      WHERE node_id = {id} AND collection_name = {collection};
    )";
    const std::string query =
        fmt::format(query_template,       //
                    fmt::arg("id", id_),  //
                    fmt::arg("collection", collection_name));
    RETURN_IF_ERROR(ExecuteQuery("SetBlackBoxLineageTrue", query));

    for (const std::string& lineage_command : lineage_commands) {
      RETURN_IF_ERROR(ExecuteQuery("LineageCommand", lineage_command));
    }

    return common::Status::OK;
  }

  // TODO(mwhittaker): Escape python_file string.
  WARN_UNUSED common::Status RegisterBlackBoxPythonLineageScript(
      const std::string& script) {
    // The 'E' makes the python_lineage_file an escaped string. See
    // https://stackoverflow.com/a/26638775/3187068 for details.
    const std::string query_template = R"(
      UPDATE Nodes
      SET python_lineage_script = E{script}
      WHERE id = {id};
    )";
    const std::string query = fmt::format(query_template,  //
                                          fmt::arg("script", SqlValue(script)),
                                          fmt::arg("id", SqlValue(id_)));
    RETURN_IF_ERROR(ExecuteQuery("SetBlackBoxPythonLineageScript", query));
    return common::Status::OK;
  }

  WARN_UNUSED common::Status RegisterBlackBoxPythonLineage(
      const std::string& collection_name, const std::string& method) {
    const std::string query_template = R"(
      UPDATE Collections
      SET lineage_type = 'python', python_lineage_method = {method}
      WHERE node_id = {id} AND collection_name = {collection};
    )";
    const std::string query =
        fmt::format(query_template,                        //
                    fmt::arg("method", SqlValue(method)),  //
                    fmt::arg("id", SqlValue(id_)),
                    fmt::arg("collection", SqlValue(collection_name)));
    RETURN_IF_ERROR(ExecuteQuery("SetBlackBoxPythonLineage", query));
    return common::Status::OK;
  }

 protected:
  InjectablePqxxClient(std::string name, std::size_t id, std::string address,
                       const ConnectionConfig& connection_config)
      : connection_(std::make_unique<Connection>(connection_config.ToString())),
        name_(std::move(name)),
        id_(id),
        address_(std::move(address)) {
    LOG(INFO)
        << "Established a lineagedb connection with the following parameters: "
        << connection_config.ToString();
  }

  // TODO(mwhittaker): Handle hash collisions.
  WARN_UNUSED common::Status Init() {
    const std::string q0_template = R"(
      INSERT INTO Nodes (id, name, address, python_lineage_script)
      VALUES ({id}, {name}, {address}, NULL);
    )";
    const std::string q0 = fmt::format(q0_template,  //
                                       fmt::arg("id", SqlValue(id_)),
                                       fmt::arg("name", SqlValue(name_)),
                                       fmt::arg("address", SqlValue(address_)));
    RETURN_IF_ERROR(ExecuteQuery("Init", q0));

    const std::string q1_template = R"(
      CREATE TABLE {name}_lineage (
        dep_node_id          {int64_type}   NOT NULL,
        dep_collection_name  {string_type}  NOT NULL,
        dep_tuple_hash       {sizet_type}   NOT NULL,
        dep_time             {int_type}     NOT NULL,
        rule_number          {int_type},
        inserted             {boolean_type} NOT NULL,
        physical_time        {time_type},
        collection_name      {string_type}  NOT NULL,
        tuple_hash           {sizet_type}   NOT NULL,
        time                 {int_type}     NOT NULL
      );
    )";
    const std::string q1 =
        fmt::format(q1_template,  //
                    fmt::arg("name", name_),
                    fmt::arg("int64_type", ToSql<std::int64_t>().Type()),
                    fmt::arg("string_type", ToSql<std::string>().Type()),
                    fmt::arg("sizet_type", ToSql<std::size_t>().Type()),
                    fmt::arg("int_type", ToSql<int>().Type()),
                    fmt::arg("boolean_type", ToSql<bool>().Type()),
                    fmt::arg("time_type", ToSql<time_point>().Type()));
    return ExecuteQuery("CreateLineageTable", q1);
  }

  // Transactionally execute the query `query` named `name`.
  virtual WARN_UNUSED common::Status ExecuteQuery(const std::string& name,
                                                  const std::string& query) {
    try {
      Work txn(*connection_, name);
      VLOG(1) << "Executing query: " << query;
      txn.exec(query);
      txn.commit();
      return common::Status::OK;
    } catch (const pqxx::pqxx_exception& e) {
      return common::Status(common::ErrorCode::INVALID_ARGUMENT,
                            e.base().what());
    }
  }

  Connection& GetConnection() { return *connection_; }

 private:
  template <typename T>
  using Type = typename ToSqlType<ToSql>::template type<T>;

  // SqlTypes<T1, ... Tn> returns the vector
  // [ToSql<T1>.Type(), ..., ToSql<Tn>().Type()]
  template <typename... Ts>
  std::vector<std::string> SqlTypes() {
    return common::TupleToVector(
        common::TypeListMapToTuple<common::TypeList<Ts...>, Type>()());
  }

  // SqlValue(x: t) is a shorthand for ToSql<t>().Value(x);
  template <typename T>
  std::string SqlValue(const T& x) {
    return ToSql<typename std::decay<decltype(x)>::type>().Value(x);
  }

  // SqlValues((a, ..., z)) returns the vector [SqlValue(a), ..., SqlValue(z)].
  template <typename... Ts>
  std::vector<std::string> SqlValues(const std::tuple<Ts...>& t) {
    return common::TupleToVector(common::TupleMap(
        t, [this](const auto& x) { return this->SqlValue(x); }));
  }

  // A connection to lineage database. Note that we'd like InjectablePqxxClient
  // to be default constructible so that we can return a
  // common::StatusOr<InjectablePqxxClient>, but a pqxx::connection is not
  // default
  // constructible. We make connection_ a unique pointer so that
  // InjectablePqxxClient is default constructible even if Connection is not.
  std::unique_ptr<Connection> connection_;

  // The name of the fluent node that owns this client.
  const std::string name_;

  // The id of the fluent node that owns this client.
  const std::int64_t id_;

  // The address of the fluent program that owns this client.
  const std::string address_;
};

// See InjectablePqxxClient documentation above.
template <template <typename> class Hash, template <typename> class ToSql,
          typename Clock>
using PqxxClient =
    InjectablePqxxClient<pqxx::connection, pqxx::work, Hash, ToSql, Clock>;

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_PQXX_CLIENT_H_
