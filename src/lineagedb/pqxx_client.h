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
namespace detail {

inline std::int64_t size_t_to_int64(std::size_t hash) {
  return static_cast<std::int64_t>(hash);
}

}  // namespace detail

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
  DISALLOW_COPY_AND_ASSIGN(InjectablePqxxClient);
  InjectablePqxxClient(InjectablePqxxClient&&) = default;
  InjectablePqxxClient& operator=(InjectablePqxxClient&&) = default;
  virtual ~InjectablePqxxClient() = default;

  static WARN_UNUSED StatusOr<InjectablePqxxClient> Make(
      std::string name, std::size_t id, std::string address,
      const ConnectionConfig& connection_config) {
    try {
      InjectablePqxxClient client(std::move(name), id, std::move(address),
                                  connection_config);
      RETURN_IF_ERROR(client.Init());
      return std::move(client);
    } catch (const pqxx::pqxx_exception& e) {
      return Status(ErrorCode::INVALID_ARGUMENT, e.base().what());
    }
  }

  template <typename... Ts>
  WARN_UNUSED Status AddCollection(
      const std::string& collection_name, const std::string& collection_type,
      const std::array<std::string, sizeof...(Ts)>& column_names) {
    static_assert(sizeof...(Ts) > 0, "Collections should have >= 1 column.");

    // For a fluent node `n` and collection `c`, we create a lineagedb relation
    // `n_c`. We also make a relation for the lineage of `n` called
    // `n_lineage`. Thus, we have a naming conflict if `c == lineage`.
    if (collection_name == "lineage") {
      return Status(ErrorCode::INVALID_ARGUMENT,
                    "Lineage is a reserved collection name.");
    }

    RETURN_IF_ERROR(ExecuteQuery(
        "AddCollection",
        fmt::format(
            R"(
      INSERT INTO Collections (node_id, collection_name, collection_type,
                               column_names, lineage_type,
                               python_lineage_method)
      VALUES ({}, 'regular', NULL);
    )",
            Join(SqlValues(std::make_tuple(id_, collection_name,
                                           collection_type, column_names))))));

    std::vector<std::string> types = SqlTypes<Ts...>();
    std::vector<std::string> columns;
    for (std::size_t i = 0; i < types.size(); ++i) {
      columns.push_back(
          fmt::format("{} {} NOT NULL", column_names[i], types[i]));
    }
    return ExecuteQuery("AddCollectionTable",
                        fmt::format(R"(
      CREATE TABLE {}_{} (
        hash                   bigint                   NOT NULL,
        time_inserted          integer                  NOT NULL,
        time_deleted           integer,
        physical_time_inserted timestamp with time zone NOT NULL,
        physical_time_deleted  timestamp with time zone,
        {},
        PRIMARY KEY (hash, time_inserted)
      );
    )",
                                    name_, collection_name, Join(columns)));
  }

  WARN_UNUSED Status AddRule(std::size_t rule_number, bool is_bootstrap,
                             const std::string& rule_string) {
    return ExecuteQuery(
        "AddRule",
        fmt::format(R"(
      INSERT INTO Rules (node_id, rule_number, is_bootstrap, rule)
      VALUES ({});
    )",
                    Join(SqlValues(std::make_tuple(
                        id_, rule_number, is_bootstrap, rule_string)))));
  }

  template <typename... Ts>
  WARN_UNUSED Status
  InsertTuple(const std::string& collection_name, int time_inserted,
              const std::chrono::time_point<Clock>& physical_time_inserted,
              const std::tuple<Ts...>& t) {
    static_assert(sizeof...(Ts) > 0, "Collections should have >=1 column.");
    std::int64_t hash = detail::size_t_to_int64(Hash<std::tuple<Ts...>>()(t));
    return ExecuteQuery(
        "InsertTuple",
        fmt::format(R"(
      INSERT INTO {}_{}
      VALUES ({}, {}, NULL, {}, NULL, {});
    )",
                    name_, collection_name, SqlValue(hash),
                    SqlValue(time_inserted), SqlValue(physical_time_inserted),
                    Join(SqlValues(t))));
  }

  template <typename... Ts>
  WARN_UNUSED Status
  DeleteTuple(const std::string& collection_name, int time_deleted,
              const std::chrono::time_point<Clock>& physical_time_deleted,
              const std::tuple<Ts...>& t) {
    static_assert(sizeof...(Ts) > 0, "Collections should have >=1 column.");
    std::int64_t hash = detail::size_t_to_int64(Hash<std::tuple<Ts...>>()(t));
    return ExecuteQuery(
        "DeleteTuple",
        fmt::format(R"(
      UPDATE {}_{}
      SET time_deleted = {}, physical_time_deleted = {}
      WHERE hash = {} AND time_deleted IS NULL;
    )",
                    name_, collection_name, SqlValue(time_deleted),
                    SqlValue(physical_time_deleted), SqlValue(hash)));
  }

  // TODO(mwhittaker): Add physical time to network lineage.
  WARN_UNUSED Status AddNetworkedLineage(std::size_t dep_node_id, int dep_time,
                                         const std::string& collection_name,
                                         std::size_t tuple_hash, int time) {
    return ExecuteQuery(
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

  WARN_UNUSED Status
  AddDerivedLineage(const LocalTupleId& dep_id, int rule_number, bool inserted,
                    const std::chrono::time_point<Clock>& physical_time,
                    const LocalTupleId& id) {
    auto values = std::make_tuple(
        detail::size_t_to_int64(id_), dep_id.collection_name,
        detail::size_t_to_int64(dep_id.hash), dep_id.logical_time_inserted,
        rule_number, inserted, physical_time, id.collection_name,
        detail::size_t_to_int64(id.hash), id.logical_time_inserted);
    return ExecuteQuery("AddLineage",
                        fmt::format(R"(
      INSERT INTO {}_lineage (dep_node_id, dep_collection_name, dep_tuple_hash,
                              dep_time, rule_number, inserted, physical_time,
                              collection_name, tuple_hash, time)
      VALUES ({});
    )",
                                    name_, Join(SqlValues(std::move(values)))));
  }

  WARN_UNUSED Status
  RegisterBlackBoxLineage(const std::string& collection_name,
                          const std::vector<std::string>& lineage_commands) {
    const std::string query_template = R"(
      UPDATE Collections
      SET lineage_type = 'sql'
      WHERE node_id = {} AND collection_name = {};
    )";
    const std::string query =
        fmt::format(query_template, SqlValue(detail::size_t_to_int64(id_)),
                    SqlValue(collection_name));
    RETURN_IF_ERROR(ExecuteQuery("SetBlackBoxLineageTrue", query));

    for (const std::string& lineage_command : lineage_commands) {
      RETURN_IF_ERROR(ExecuteQuery("LineageCommand", lineage_command));
    }

    return Status::OK;
  }

  // TODO(mwhittaker): Escape python_file string.
  WARN_UNUSED Status
  RegisterBlackBoxPythonLineageScript(const std::string& script) {
    // The 'E' makes the python_lineage_file an escaped string. See
    // https://stackoverflow.com/a/26638775/3187068 for details.
    const std::string query_template = R"(
      UPDATE Nodes
      SET python_lineage_script = E{}
      WHERE node_id = {};
    )";
    const std::string query =
        fmt::format(query_template, SqlValue(script),
                    SqlValue(detail::size_t_to_int64(id_)));
    RETURN_IF_ERROR(ExecuteQuery("SetBlackBoxPythonLineageScript", query));
    return Status::OK;
  }

  WARN_UNUSED Status RegisterBlackBoxPythonLineage(
      const std::string& collection_name, const std::string& method) {
    const std::string query_template = R"(
      UPDATE Collections
      SET python_lineage_method = {}
      WHERE node_id = {} AND collection_name = {};
    )";
    const std::string query = fmt::format(
        query_template, SqlValue(method),
        SqlValue(detail::size_t_to_int64(id_)), SqlValue(collection_name));
    RETURN_IF_ERROR(ExecuteQuery("SetBlackBoxPythonLineage", query));
    return Status::OK;
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
  WARN_UNUSED Status Init() {
    RETURN_IF_ERROR(ExecuteQuery(
        "Init",
        fmt::format(R"(
      INSERT INTO Nodes (id, name, address, python_lineage_script)
      VALUES ({}, NULL);
    )",
                    Join(SqlValues(std::make_tuple(id_, name_, address_))))));

    return ExecuteQuery("CreateLineageTable", fmt::format(R"(
      CREATE TABLE {}_lineage (
        dep_node_id          bigint                    NOT NULL,
        dep_collection_name  text                      NOT NULL,
        dep_tuple_hash       bigint                    NOT NULL,
        dep_time             bigint                    NOT NULL,
        rule_number          integer,
        inserted             boolean                   NOT NULL,
        physical_time        timestamp with time zone,
        collection_name      text                      NOT NULL,
        tuple_hash           bigint                    NOT NULL,
        time                 integer                   NOT NULL
      );
    )",
                                                          name_));
  }

  // Transactionally execute the query `query` named `name`.
  virtual WARN_UNUSED Status ExecuteQuery(const std::string& name,
                                          const std::string& query) {
    try {
      Work txn(*connection_, name);
      VLOG(1) << "Executing query: " << query;
      txn.exec(query);
      txn.commit();
      return Status::OK;
    } catch (const pqxx::pqxx_exception& e) {
      return Status(ErrorCode::INVALID_ARGUMENT, e.base().what());
    }
  }

 private:
  template <typename T>
  using Type = typename ToSqlType<ToSql>::template type<T>;

  // SqlTypes<T1, ... Tn> returns the vector
  // [ToSql<T1>.Type(), ..., ToSql<Tn>().Type()]
  template <typename... Ts>
  std::vector<std::string> SqlTypes() {
    return TupleToVector(TypeListMapToTuple<TypeList<Ts...>, Type>()());
  }

  // SqlValue(x: t) is a shorthand for ToSql<t>().Value(x);
  template <typename T>
  std::string SqlValue(const T& x) {
    return ToSql<typename std::decay<decltype(x)>::type>().Value(x);
  }

  // SqlValues((a, ..., z)) returns the vector [SqlValue(a), ..., SqlValue(z)].
  template <typename... Ts>
  std::vector<std::string> SqlValues(const std::tuple<Ts...>& t) {
    return TupleToVector(
        TupleMap(t, [this](const auto& x) { return this->SqlValue(x); }));
  }

  // A connection to lineage database. Note that we'd like InjectablePqxxClient
  // to be default constructible so that we can return a
  // StatusOr<InjectablePqxxClient>, but a pqxx::connection is not default
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
