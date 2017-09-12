#ifndef FLUENT_FLUENT_EXECUTOR_H_
#define FLUENT_FLUENT_EXECUTOR_H_

#include <cstddef>
#include <cstdint>

#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "zmq.hpp"

#include "collections/all.h"
#include "collections/collection_util.h"
#include "common/macros.h"
#include "common/static_assert.h"
#include "common/status.h"
#include "common/status_macros.h"
#include "common/status_or.h"
#include "common/string_util.h"
#include "common/tuple_util.h"
#include "common/type_list.h"
#include "fluent/network_state.h"
#include "fluent/rule.h"
#include "fluent/rule_tags.h"
#include "fluent/timestamp_wrapper.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/to_sql.h"
#include "ra/logical_to_physical.h"
#include "zmq_util/socket_cache.h"

namespace fluent {
namespace detail {

// IsRuleTagInsert
template <typename RuleTag>
struct IsRuleTagInsert;

template <>
struct IsRuleTagInsert<MergeTag> : public std::true_type {};

template <>
struct IsRuleTagInsert<DeferredMergeTag> : public std::true_type {};

template <>
struct IsRuleTagInsert<DeferredDeleteTag> : public std::false_type {};

// UpdateCollection
template <typename Collection, typename... Ts>
void UpdateCollection(Collection* collection, const std::tuple<Ts...>& t,
                      std::size_t hash, int logical_time_inserted, MergeTag) {
  collection->Merge(t, hash, logical_time_inserted);
}

template <typename Collection, typename... Ts>
void UpdateCollection(Collection* collection, const std::tuple<Ts...>& t,
                      std::size_t hash, int logical_time_inserted,
                      DeferredMergeTag) {
  collection->DeferredMerge(t, hash, logical_time_inserted);
}

template <typename Collection, typename... Ts>
void UpdateCollection(Collection* collection, const std::tuple<Ts...>& t,
                      std::size_t hash, int logical_time_inserted,
                      DeferredDeleteTag) {
  collection->DeferredDelete(t, hash, logical_time_inserted);
}

// ProcessChannel
template <typename Collection>
struct ProcessChannelImpl {
  template <typename F>
  common::Status operator()(Collection* c, F f) {
    UNUSED(f);
    UNUSED(c);
    return common::Status::OK;
  }
};

template <template <typename> class Pickler, typename T, typename... Ts>
struct ProcessChannelImpl<collections::Channel<Pickler, T, Ts...>> {
  template <typename F>
  common::Status operator()(collections::Channel<Pickler, T, Ts...>* c, F f) {
    return f(c);
  }
};

template <typename Collection, typename F>
common::Status ProcessChannel(Collection* c, F f) {
  return ProcessChannelImpl<Collection>()(c, f);
}

}  // namespace detail

// See below.
template <typename Collections, typename BootstrapRules, typename Rules,
          template <template <typename> class Hash,
                    template <typename> class ToSql, typename Clock>
          class LineageDbClient,
          template <typename> class Hash, template <typename> class ToSql,
          template <typename> class Pickler, typename Clock>
class FluentExecutor;

template <typename... Collections, typename... BootstrapCollections,
          typename... BootstrapRuleTags, typename... BootstrapRas,
          typename... RuleCollections, typename... RuleTags, typename... Ras,
          template <template <typename> class Hash,
                    template <typename> class ToSql, typename Clock>
          class LineageDbClient,
          template <typename> class Hash, template <typename> class ToSql,
          template <typename> class Pickler, typename Clock>
class FluentExecutor<common::TypeList<Collections...>,
                     common::TypeList<Rule<BootstrapCollections,
                                           BootstrapRuleTags, BootstrapRas>...>,
                     common::TypeList<Rule<RuleCollections, RuleTags, Ras>...>,
                     LineageDbClient, Hash, ToSql, Pickler, Clock> {
 public:
  using BootstrapRules = common::TypeList<
      Rule<BootstrapCollections, BootstrapRuleTags, BootstrapRas>...>;
  using Rules = common::TypeList<Rule<RuleCollections, RuleTags, Ras>...>;
  using BootstrapRulesTuple =
      typename common::TypeListToTuple<BootstrapRules>::type;
  using RulesTuple = typename common::TypeListToTuple<Rules>::type;
  using Time = std::chrono::time_point<Clock>;
  using PeriodicId = typename collections::Periodic<Clock>::id;

  static WARN_UNUSED common::StatusOr<FluentExecutor> Make(
      std::string name, std::size_t id,
      std::tuple<std::unique_ptr<Collections>...> collections,
      BootstrapRulesTuple bootstrap_rules,
      std::unique_ptr<TimestampWrapper> logical_time_wrapper,
      std::unique_ptr<NetworkState> network_state, collections::Stdin* stdin,
      std::vector<collections::Periodic<Clock>*> periodics,
      std::unique_ptr<LineageDbClient<Hash, ToSql, Clock>> lineagedb_client,
      RulesTuple rules) {
    FluentExecutor f(
        std::move(name), id, std::move(collections), std::move(bootstrap_rules),
        std::move(logical_time_wrapper), std::move(network_state), stdin,
        std::move(periodics), std::move(lineagedb_client), std::move(rules));
    RETURN_IF_ERROR(f.InitLineageDbClient());
    return std::move(f);
  }

  FluentExecutor(
      std::string name, std::size_t id,
      std::tuple<std::unique_ptr<Collections>...> collections,
      BootstrapRulesTuple bootstrap_rules,
      std::unique_ptr<TimestampWrapper> logical_time_wrapper,
      std::unique_ptr<NetworkState> network_state, collections::Stdin* stdin,
      std::vector<collections::Periodic<Clock>*> periodics,
      std::unique_ptr<LineageDbClient<Hash, ToSql, Clock>> lineagedb_client,
      RulesTuple rules)
      : name_(std::move(name)),
        id_(id),
        collections_(std::move(collections)),
        bootstrap_rules_(std::move(bootstrap_rules)),
        logical_time_wrapper_(std::move(logical_time_wrapper)),
        network_state_(std::move(network_state)),
        stdin_(stdin),
        periodics_(std::move(periodics)),
        lineagedb_client_(std::move(lineagedb_client)),
        rules_(rules) {
    // Initialize periodic timeouts. See the comment above `PeriodicTimeout`
    // below for more information.
    Time now = Clock::now();
    for (collections::Periodic<Clock>* p : periodics_) {
      timeout_queue_.push(PeriodicTimeout{now + p->Period(), p});
    }
  }
  DISALLOW_COPY_AND_ASSIGN(FluentExecutor);
  DEFAULT_MOVE_AND_ASSIGN(FluentExecutor);

  // Get<I>() returns a const reference to the Ith collection.
  template <std::size_t I>
  const auto& Get() const {
    return *std::get<I>(collections_);
  }

  // Return a reference to a FluentExecutor's Lineage DB client. When
  // LineageDbClient is MockClient, this method allows us to unit test that
  // FluentExecutors are properly tracking lineage and history. See
  // fluent_executor_test.cc for some examples.
  const LineageDbClient<Hash, ToSql, Clock>& GetLineageDbClient() {
    return *lineagedb_client_.get();
  }

  // Refer to [1] for an overview of black box lineage. The
  // RegisterBlackBoxLineage method is used to specify the lineage of a black
  // box function. It helps to look at an example. Consider wrapping a simple
  // key-value store with the following API:
  //
  //   - set(key: string, value: string) -> (suceeded: bool)
  //   - get(key: string) -> (value: string)
  //
  // First, we write a Fluent shim with the following channels:
  //
  //   fluent("redis", ...)
  //     .channel<string, string, int64_t, string, string>(
  //       "set_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
  //     .channel<string, int64_t, bool>(
  //       "set_response", {{"addr", "id", "succeeded"}})
  //     .channel<string, string, int64_t, string>(
  //       "get_request", {{"dst_addr", "src_addr", "id", "key"}})
  //     .channel<string, int64_t, string>(
  //       "get_response", {{"addr", "id", "value"}})
  //     .RegisterRules(...);
  //
  // Fluent will automatically generate the following relations in postgres:
  //
  //   CREATE TABLE SetRequest (
  //       hash          integer             NOT NULL,
  //       time_inserted integer             NOT NULL,
  //       time_deleted  integer             NOT NULL,
  //       physical_time_inserted timestampz NOT NULL,
  //       physical_time_deleted  timestampz NOT NULL,
  //       dst_addr      text                NOT NULL,
  //       src_addr      text                NOT NULL,
  //       id            integer             NOT NULL,
  //       key           text                NOT NULL,
  //       value         text                NOT NULL,
  //       PRIMARY KEY (hash, time_inserted)
  //   );
  //
  //   CREATE TABLE SetResponse (
  //       hash          integer             NOT NULL,
  //       time_inserted integer             NOT NULL,
  //       time_deleted  integer             NOT NULL,
  //       physical_time_inserted timestampz NOT NULL,
  //       physical_time_deleted  timestampz NOT NULL,
  //       addr          text                NOT NULL,
  //       id            integer             NOT NULL,
  //       succeeded     boolean             NOT NULL,
  //       PRIMARY KEY (hash, time_inserted)
  //   );
  //
  //   CREATE TABLE GetRequest (
  //       hash          integer             NOT NULL,
  //       time_inserted integer             NOT NULL,
  //       time_deleted  integer             NOT NULL,
  //       physical_time_inserted timestampz NOT NULL,
  //       physical_time_deleted  timestampz NOT NULL,
  //       dst_addr      text                NOT NULL,
  //       src_addr      text                NOT NULL,
  //       id            integer             NOT NULL,
  //       key           text                NOT NULL,
  //       PRIMARY KEY (hash, time_inserted)
  //   );
  //
  //   CREATE TABLE GetResponse (
  //       hash          integer             NOT NULL,
  //       time_inserted integer             NOT NULL,
  //       time_deleted  integer             NOT NULL,
  //       physical_time_inserted timestampz NOT NULL,
  //       physical_time_deleted  timestampz NOT NULL,
  //       addr          text                NOT NULL,
  //       id            integer             NOT NULL,
  //       value         text                NOT NULL,
  //       PRIMARY KEY (hash, time_inserted)
  //   );
  //
  // We want to register the following two functions which specify the lineage
  // of get responses. Note that results are returned in descending order of
  // physical time.
  //
  //   -- redis_get_response_lineage_impl(time_inserted, key, value)
  //   CREATE redis_get_response_lineage_impl(integer, text, text)
  //   RETURNS TABLE(node_name text,
  //                 collection_name text,
  //                 hash integer,
  //                 time_inserted integer,
  //                 physical_time_inserted timestampz)
  //   AS $$
  //     SELECT
  //       CAST('redis_server' AS TEXT),
  //       CAST('set_request' AS text),
  //       hash,
  //       time_inserted,
  //       physical_time_inserted
  //     FROM redis_server_set_request
  //     WHERE time_inserted <= $1 AND key = $2
  //     ORDER BY time_inserted DESC
  //     LIMIT 1
  //   $$ LANGUAGE SQL;
  //
  //   -- id
  //   get_response_lineage(id)
  //   CREATE FUNCTION redis_get_response_lineage(integer)
  //   RETURNS TABLE(node_name text,
  //                 collection_name text,
  //                 hash integer,
  //                 time_inserted integer)
  //   AS $$
  //     WITH
  //     user_specified_lineage(node_name,
  //                            collection_name,
  //                            hash,
  //                            time_inserted,
  //                            physical_time_inserted) AS (
  //       SELECT L.*
  //       FROM redis_get_request Req,
  //            redis_get_response Resp,
  //            redis_get_response_lineage_impl(
  //                Req.time_inserted, Req.key, Resp.value) AS L
  //       WHERE Req.id = $1 AND Resp.id = $1
  //     ),
  //
  //     request_lineage(node_name,
  //                     collection_name,
  //                     hash,
  //                     time_inserted,
  //                     physical_time_inserted) AS (
  //       SELECT
  //         CAST('redis_server' as text),
  //         CAST('get_request' as text),
  //         hash,
  //         time_inserted,
  //         physical_time_inserted
  //       FROM redis_server_get_request
  //       WHERE id = $1;
  //     ),
  //
  //     lineage AS (
  //       SELECT * FROM user_specified_lineage UNION
  //       SELECT * FROM request_lineage
  //     )
  //
  //     SELECT node_name, collection_name, hash, time_inserted
  //     FROM lineage
  //     ORDER BY physical_time_inserted DESC
  //   $$ LANGUAGE SQL;
  //
  // These two functions are mostly boilerplate. The only creative bit that a
  // user would have to provide is this part:
  //
  //   SELECT
  //     CAST('redis_server' AS TEXT),
  //     CAST('set_request' AS text),
  //     hash,
  //     time_inserted,
  //     physical_time_inserted
  //   FROM redis_server_set_request
  //   WHERE time_inserted <= $1 AND key = $2
  //   ORDER BY time_inserted DESC
  //   LIMIT 1
  //
  // RegisterBlackBoxLineage<ReqIndex, RespIndex>(f) takes a function f that
  // returns this lineage specification. f takes one argument for every
  // argument of the first SQL function. For example, here's what f would look
  // like to generate the SQL above:
  //
  //   [](const string& time_inserted, const string& key, const string& value) {
  //     return fmt::format(R"(
  //       SELECT
  //         CAST('redis_server' AS TEXT),
  //         CAST('set_request' AS text),
  //         hash,
  //         time_inserted,
  //         physical_time_inserted
  //       FROM redis_server_set_request
  //       WHERE time_inserted <= {} AND key = {}
  //       ORDER BY time_inserted DESC
  //       LIMIT 1
  //     )", time_inserted, key);
  //   }
  //
  // ReqIndex and RespIndex specify the request and response channel whose
  // lineage is being specified. For example, given the following fluent
  // program:
  //
  //   auto f = fluent("some_server", ...)
  //     .channel<...>("foo_request", ...)
  //     .channel<...>("foo_response", ...)
  //     .channel<...>("bar_request", ...)
  //     .channel<...>("bar_response", ...)
  //     .RegisterRules(...);
  //
  // we can specify the lineage of the foo API with
  // f.RegisterBlackBoxLineage<0, 1>(...) and specify the lineage of the bar
  // API with f.RegisterBlackBoxLineage<2, 3>(...).
  //
  // [1]: https://goo.gl/yGmr78
  template <std::size_t RequestIndex, std::size_t ResponseIndex, typename F>
  WARN_UNUSED common::Status RegisterBlackBoxLineage(F f) {
    ValidateBlackBoxIndexes<RequestIndex, ResponseIndex>();
    return RegisterBlackBoxLineageImpl(Get<RequestIndex>(),
                                       Get<ResponseIndex>(), f);
  }

  // TODO(mwhittaker): Document.
  WARN_UNUSED common::Status RegisterBlackBoxPythonLineageScript(
      const std::string& script) {
    return lineagedb_client_->RegisterBlackBoxPythonLineageScript(script);
  }

  // TODO(mwhittaker): Document.
  template <std::size_t RequestIndex, std::size_t ResponseIndex>
  WARN_UNUSED common::Status RegisterBlackBoxPythonLineage(
      const std::string& method) {
    ValidateBlackBoxIndexes<RequestIndex, ResponseIndex>();
    const std::string collection_name = Get<ResponseIndex>().Name();
    return lineagedb_client_->RegisterBlackBoxPythonLineage(collection_name,
                                                            method);
  }

  // Sequentially execute each registered bootstrap query and then invoke the
  // `Tick` method of every collection.
  WARN_UNUSED common::Status BootstrapTick() {
    if (common::TypeListLen<BootstrapRules>::value == 0) {
      return common::Status::OK;
    }

    RETURN_IF_ERROR(common::TupleIteriStatus(
        bootstrap_rules_, [this](std::size_t rule_number, auto& rule) {
          return this->ExecuteRule(rule_number, &rule);
        }));
    time_++;
    return common::TupleIterStatus(collections_, [this](auto& c) {
      return this->TickCollection(c.get());
    });
  }

  // Sequentially execute each registered query and then invoke the `Tick`
  // method of every collection.
  WARN_UNUSED common::Status Tick() {
    RETURN_IF_ERROR(common::TupleIteriStatus(
        rules_, [this](std::size_t rule_number, auto& rule) {
          return this->ExecuteRule(rule_number, &rule);
        }));
    time_++;
    return common::TupleIterStatus(collections_, [this](auto& c) {
      return this->TickCollection(c.get());
    });
  }

  // (Potentially) block and receive messages sent by other Fluent nodes.
  // Receiving a message will insert it into the appropriate channel.
  WARN_UNUSED common::Status Receive() {
    time_++;

    zmq::pollitem_t sock_pollitem = {static_cast<void*>(network_state_->socket),
                                     0, ZMQ_POLLIN, 0};
    std::vector<zmq::pollitem_t> pollitems = {sock_pollitem};
    if (stdin_ != nullptr) {
      pollitems.push_back(stdin_->Pollitem());
    }

    long timeout = GetPollTimeoutInMicros();
    zmq_util::poll(timeout, &pollitems);

    // Read from the network.
    if (pollitems[0].revents & ZMQ_POLLIN) {
      // msgs[0] = dep node id
      // msgs[1] = dep collection name
      // msgs[2] = dep time
      // msgs[3] = tuple element 0
      // msgs[4] = tuple element 1
      // ...
      std::vector<zmq::message_t> msgs =
          zmq_util::recv_msgs(&network_state_->socket);

      std::vector<std::string> strings;
      for (std::size_t i = 3; i < msgs.size(); ++i) {
        strings.push_back(zmq_util::message_to_string(msgs[i]));
      }

      const std::string dep_node_id_str = zmq_util::message_to_string(msgs[0]);
      const std::string channel_name_str = zmq_util::message_to_string(msgs[1]);
      const std::string dep_time_str = zmq_util::message_to_string(msgs[2]);
      const std::size_t dep_node_id =
          Pickler<std::size_t>().Load(dep_node_id_str);
      const std::string channel_name =
          Pickler<std::string>().Load(channel_name_str);
      const int dep_time = Pickler<int>().Load(dep_time_str);

      RETURN_IF_ERROR(common::TupleIterStatus(
          collections_,  //
          [&, dep_node_id, dep_time](auto& collection_ptr) {
            return detail::ProcessChannel(
                collection_ptr.get(),
                [&, dep_node_id, dep_time](auto* channel) {
                  if (channel->Name() != channel_name) {
                    return common::Status::OK;
                  }

                  const auto t = channel->Parse(strings);
                  Hash<typename std::decay<decltype(t)>::type> hash;
                  channel->Receive(t, hash(t), time_);
                  RETURN_IF_ERROR(lineagedb_client_->InsertTuple(
                      channel->Name(), time_, Clock::now(), t));
                  RETURN_IF_ERROR(lineagedb_client_->AddNetworkedLineage(
                      dep_node_id, dep_time, channel->Name(), hash(t), time_));
                  return common::Status::OK;

                });
          }));
    }

    // Read from stdin.
    if (stdin_ != nullptr && pollitems[1].revents & ZMQ_POLLIN) {
      const std::tuple<std::string> line = stdin_->ReadLine();
      stdin_->Merge(line, Hash<std::tuple<std::string>>()(line), time_);
      RETURN_IF_ERROR(lineagedb_client_->InsertTuple(stdin_->Name(), time_,
                                                     Clock::now(), line));
    }

    // Trigger periodics.
    RETURN_IF_ERROR(TockPeriodics());

    return common::Status::OK;
  }

  // Runs a fluent program.
  WARN_UNUSED common::Status Run() {
    RETURN_IF_ERROR(BootstrapTick());
    while (true) {
      RETURN_IF_ERROR(Receive());
      RETURN_IF_ERROR(Tick());
    }
  }

 protected:
  // MutableGet<I>() returns a reference to the Ith collection. This should
  // only really ever be used for unit testing.
  template <std::size_t I>
  auto& MutableGet() const {
    return *std::get<I>(collections_);
  }

 private:
  // Initialize a node with the lineagedb database.
  WARN_UNUSED common::Status InitLineageDbClient() {
    // Collections.
    RETURN_IF_ERROR(
        common::TupleIterStatus(collections_, [this](const auto& collection) {
          // collection is of type `const unique_ptr<collection_type>&`.
          using collection_type = typename common::Unwrap<
              typename std::decay<decltype(collection)>::type>::type;
          return this->AddCollection(
              collection,
              typename collections::CollectionTypes<collection_type>::type{});
        }));

    // Rules.
    RETURN_IF_ERROR(common::TupleIteriStatus(
        bootstrap_rules_, [this](std::size_t i, const auto& bootstrap_rule) {
          return lineagedb_client_->AddRule(i, true,
                                            bootstrap_rule.ToDebugString());
        }));
    RETURN_IF_ERROR(common::TupleIteriStatus(
        rules_, [this](std::size_t i, const auto& rule) {
          return lineagedb_client_->AddRule(i, false, rule.ToDebugString());
        }));

    return common::Status::OK;
  }

  // Register a collection with the lineagedb database.
  template <typename Collection, typename... Ts>
  WARN_UNUSED common::Status AddCollection(const std::unique_ptr<Collection>& c,
                                           common::TypeList<Ts...>) {
    return lineagedb_client_->template AddCollection<Ts...>(
        c->Name(), CollectionTypeToString(
                       collections::GetCollectionType<Collection>::value),
        c->ColumnNames());
  }

  // Tick a collection and insert the deleted tuples into the lineagedb
  // database.
  template <typename Collection>
  WARN_UNUSED common::Status TickCollection(Collection* c) {
    auto deleted = c->Tick();
    for (const auto& pair : deleted) {
      const auto& t = pair.first;
      RETURN_IF_ERROR(
          lineagedb_client_->DeleteTuple(c->Name(), time_, Clock::now(), t));
    }
    return common::Status::OK;
  }

  // `GetPollTimeoutInMicros` returns the minimum time (in microseconds) that we
  // need to wait before a PeriodicTimeout in `timeout_queue_` is ready. If
  // `timeout_queue_` is empty, then we return -1, which indicates that we
  // should wait forever.
  long GetPollTimeoutInMicros() {
    if (timeout_queue_.size() == 0) {
      return -1;
    }

    // The `zmq_poll` API is a bit confusing. It says "zmq_poll() shall wait
    // timeout microseconds for an event to occur" but then also says "The
    // resolution of timeout is 1 millisecond". Then, if you look at `zmq.hpp`,
    // you see `poll` takes std::chrono::milliseconds [1]. After
    // experimentation, it definitely takes milliseconds.
    //
    // [1]: http://bit.ly/2n3SqEx
    std::chrono::milliseconds timeout =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            timeout_queue_.top().timeout - Clock::now());
    return std::max<long>(0, timeout.count());
  }

  // Call `Tock` on every Periodic that's ready to be tocked. See the comment
  // on `PeriodicTimeout` down below for more information.
  WARN_UNUSED common::Status TockPeriodics() {
    Time now = Clock::now();
    while (timeout_queue_.size() != 0 && timeout_queue_.top().timeout <= now) {
      PeriodicTimeout timeout = timeout_queue_.top();
      timeout_queue_.pop();

      // TODO(mwhittaker): Make Periodic templated on Clock.
      PeriodicId id = timeout.periodic->GetAndIncrementId();
      std::tuple<PeriodicId, Time> t(id, now);
      Hash<std::tuple<PeriodicId, Time>> hash;
      timeout.periodic->Merge(t, hash(t), time_);
      RETURN_IF_ERROR(lineagedb_client_->InsertTuple(timeout.periodic->Name(),
                                                     time_, now, t));
      timeout.timeout = now + timeout.periodic->Period();
      timeout_queue_.push(timeout);
    }
    return common::Status::OK;
  }

  template <std::size_t RequestIndex, std::size_t ResponseIndex>
  void ValidateBlackBoxIndexes() {
    using num_collections = common::sizet_constant<sizeof...(Collections)>;
    using req_index = common::sizet_constant<RequestIndex>;
    using resp_index = common::sizet_constant<ResponseIndex>;
    static_assert(
        common::StaticAssert<common::Lt<req_index, num_collections>>::value,
        "RequestIndex out of bounds.");
    static_assert(
        common::StaticAssert<common::Lt<resp_index, num_collections>>::value,
        "ResponseIndex out of bounds.");
    static_assert(
        common::StaticAssert<common::Ne<req_index, resp_index>>::value,
        "The same channel cannot be simultaneously a request and a "
        "response channel.");

    using RequestType =
        typename std::decay<decltype(Get<RequestIndex>())>::type;
    using ResponseType =
        typename std::decay<decltype(Get<ResponseIndex>())>::type;
    static_assert(collections::GetCollectionType<RequestType>::value ==
                      collections::CollectionType::CHANNEL,
                  "Request collection is not a channel.");
    static_assert(collections::GetCollectionType<ResponseType>::value ==
                      collections::CollectionType::CHANNEL,
                  "Request collection is not a channel.");
  }

  // See RegisterBlackBoxLineage.
  template <typename T>
  using SqlType = typename lineagedb::ToSqlType<ToSql>::template type<T>;

  // See RegisterBlackBoxLineage.
  template <std::size_t... Is, typename F>
  std::string CallBlackBoxLineageFunction(F f, std::index_sequence<Is...>) {
    return f(("$" + std::to_string(Is + 1))...);
  }

  // See RegisterBlackBoxLineage.
  template <template <typename> class Pickler_, typename... RequestTs,
            typename... ResponseTs, typename F>
  WARN_UNUSED common::Status RegisterBlackBoxLineageImpl(
      const collections::Channel<Pickler_, RequestTs...>& request,
      const collections::Channel<Pickler_, ResponseTs...>& response, F f) {
    // Validate request types.
    if (request.ColumnNames().size() < static_cast<std::size_t>(3) ||
        request.ColumnNames()[0] != "dst_addr" ||
        request.ColumnNames()[1] != "src_addr" ||
        request.ColumnNames()[2] != "id") {
      const std::string request_columns_err =
          "The first three columns of a request channel must be a dst_addr, "
          "src_addr, and id column. The remaining columns are the arguments to "
          "the request.";
      return common::Status(common::ErrorCode::INVALID_ARGUMENT,
                            request_columns_err);
    }
    using RequestTypes = common::TypeList<RequestTs...>;
    using RequestType0 = typename common::TypeListGet<RequestTypes, 0>::type;
    using RequestType1 = typename common::TypeListGet<RequestTypes, 1>::type;
    using RequestType2 = typename common::TypeListGet<RequestTypes, 2>::type;
    using correct_req_type_0 = std::is_same<std::string, RequestType0>;
    using correct_req_type_1 = std::is_same<std::string, RequestType1>;
    using correct_req_type_2 = std::is_same<std::int64_t, RequestType2>;
    static_assert(common::StaticAssert<correct_req_type_0>::value, "");
    static_assert(common::StaticAssert<correct_req_type_1>::value, "");
    static_assert(common::StaticAssert<correct_req_type_2>::value, "");

    // Validate response types.
    if (response.ColumnNames().size() < static_cast<std::size_t>(2) ||
        response.ColumnNames()[0] != "addr" ||
        response.ColumnNames()[1] != "id") {
      const std::string response_columns_err =
          "The first two columns of a response channel must be a addr and id "
          "column. The remaining columns are the return values of the "
          "response.";
      return common::Status(common::ErrorCode::INVALID_ARGUMENT,
                            response_columns_err);
    }
    using ResponseTypes = common::TypeList<ResponseTs...>;
    using ResponseType0 = typename common::TypeListGet<ResponseTypes, 0>::type;
    using ResponseType1 = typename common::TypeListGet<ResponseTypes, 1>::type;
    using correct_resp_type_0 = std::is_same<std::string, ResponseType0>;
    using correct_resp_type_1 = std::is_same<std::int64_t, ResponseType1>;
    static_assert(common::StaticAssert<correct_resp_type_0>::value, "");
    static_assert(common::StaticAssert<correct_resp_type_1>::value, "");

    // Collect argument and return types.
    using ArgTypes = typename common::TypeListDrop<RequestTypes, 3>::type;
    using RetTypes = typename common::TypeListDrop<ResponseTypes, 2>::type;
    std::vector<std::string> types;
    auto arg_types = common::TypeListMapToTuple<ArgTypes, SqlType>()();
    auto ret_types = common::TypeListMapToTuple<RetTypes, SqlType>()();
    common::TupleIter(arg_types,
                      [&types](const auto& s) { types.push_back(s); });
    common::TupleIter(ret_types,
                      [&types](const auto& s) { types.push_back(s); });

    // Collect argument and return column names.
    std::vector<std::string> column_names;
    for (std::size_t i = 3; i < request.ColumnNames().size(); ++i) {
      const std::string& column_name = request.ColumnNames()[i];
      column_names.push_back(fmt::format("Req.{}", column_name));
    }
    for (std::size_t i = 2; i < response.ColumnNames().size(); ++i) {
      const std::string& column_name = response.ColumnNames()[i];
      column_names.push_back(fmt::format("Resp.{}", column_name));
    }

    // Issue SQL queries.
    constexpr std::size_t num_args = common::TypeListLen<ArgTypes>::value;
    constexpr std::size_t num_rets = common::TypeListLen<RetTypes>::value;
    auto seq = std::make_index_sequence<1 + num_args + num_rets>();
    const std::string lineage_impl_command = fmt::format(
        R"(
      CREATE FUNCTION {name}_{resp}_lineage_impl(integer, {types})
      RETURNS TABLE(node_name {string_type},
                    collection_name {string_type},
                    hash {sizet_type},
                    time_inserted {int_type},
                    physical_time_inserted {time_type})
      AS $${query}$$ LANGUAGE SQL;
    )",
        fmt::arg("name", name_), fmt::arg("resp", response.Name()),
        fmt::arg("int_type", ToSql<int>().Type()),
        fmt::arg("sizet_type", ToSql<std::size_t>().Type()),
        fmt::arg("string_type", ToSql<std::string>().Type()),
        fmt::arg("time_type", ToSql<Time>().Type()),
        fmt::arg("types", common::Join(types)),
        fmt::arg("query", CallBlackBoxLineageFunction(f, seq)));

    const std::string lineage_command = fmt::format(
        R"(
      CREATE FUNCTION {node_name}_{response_table}_lineage({int64_type})
      RETURNS TABLE(node_name {string_type},
                    collection_name {string_type},
                    hash {sizet_type},
                    time_inserted {int_type})
      AS $$
        WITH
        user_specified_lineage(node_name,
                               collection_name,
                               hash,
                               time_inserted,
                               physical_time_inserted) AS (
          SELECT L.*
          FROM {node_name}_{request_table} Req,
               {node_name}_{response_table} Resp,
               {node_name}_{response_table}_lineage_impl(
                   Req.time_inserted, {column_names}) AS L
          WHERE Req.id = $1 AND Resp.id = $1
        ),

        request_lineage(node_name,
                        collection_name,
                        hash,
                        time_inserted,
                        physical_time_inserted) AS (
          SELECT CAST('{node_name}' AS text),
                 CAST('{request_table}' AS text),
                 hash,
                 time_inserted,
                 physical_time_inserted
          FROM {node_name}_{request_table}
          WHERE id = $1
        ),

        lineage AS (
          SELECT * FROM user_specified_lineage UNION
          SELECT * FROM request_lineage
        )

        SELECT node_name, collection_name, hash, time_inserted
        FROM lineage
        ORDER BY physical_time_inserted DESC
      $$ LANGUAGE SQL;
    )",
        fmt::arg("int_type", ToSql<int>().Type()),
        fmt::arg("sizet_type", ToSql<std::size_t>().Type()),
        fmt::arg("string_type", ToSql<std::string>().Type()),
        fmt::arg("int64_type", ToSql<std::int64_t>().Type()),
        fmt::arg("time_type", ToSql<Time>().Type()),
        fmt::arg("node_name", name_), fmt::arg("request_table", request.Name()),
        fmt::arg("response_table", response.Name()),
        fmt::arg("column_names", common::Join(column_names)));

    return lineagedb_client_->RegisterBlackBoxLineage(
        response.Name(),
        {std::move(lineage_impl_command), std::move(lineage_command)});
  }

  // TODO(mwhittaker): Document.
  template <typename Collection, typename RuleTag, typename Ra>
  WARN_UNUSED common::Status ExecuteRule(int rule_number,
                                         Rule<Collection, RuleTag, Ra>* rule) {
    VLOG(1) << "Executing rule " << rule_number << ".";

    if (logical_time_wrapper_ != nullptr) {
      logical_time_wrapper_->Set(&time_);
    }
    time_++;

    using column_types = typename Ra::column_types;
    using tuple_type = typename common::TypeListToTuple<column_types>::type;
    Hash<tuple_type> hash;
    const bool is_insert = detail::IsRuleTagInsert<RuleTag>::value;

    auto phy = ra::LogicalToPhysical(rule->ra);
    auto rng = phy.ToRange();
    std::set<tuple_type> ts;
    std::chrono::time_point<Clock> physical_time = Clock::now();

    for (auto iter = ranges::begin(rng); iter != ranges::end(rng); iter++) {
      const auto tuple_and_ids = *iter;
      const auto& tuple = std::get<0>(tuple_and_ids);
      const std::set<LocalTupleId>& ids = std::get<1>(tuple_and_ids);

      // Imagine a rule like t <= make_collection(t) which feeds t back into
      // itself. We have to be careful not to insert something into t while
      // we're iterating over it. If we do, we'll invaidate our iterators.
      // Instead, we buffer the tuples and insert them down below.
      ts.insert(tuple);

      if (is_insert) {
        RETURN_IF_ERROR(lineagedb_client_->InsertTuple(
            rule->collection->Name(), time_, Clock::now(), tuple));

        switch (collections::GetCollectionType<Collection>::value) {
          case collections::CollectionType::CHANNEL:
          case collections::CollectionType::STDOUT: {
            // When a tuple is is_insert into a channel or stdout, it isn't
            // really is_insert at all. Channels send their tuples away and
            // stdout just prints the message to the screen. Thus, we insert
            // and then immediately delete the tuple.
            RETURN_IF_ERROR(lineagedb_client_->DeleteTuple(
                rule->collection->Name(), time_, Clock::now(), tuple));
          }
          case collections::CollectionType::TABLE:
          case collections::CollectionType::SCRATCH:
          case collections::CollectionType::STDIN:
          case collections::CollectionType::PERIODIC: {
            // Do nothing.
          }
        }
      } else {
        RETURN_IF_ERROR(lineagedb_client_->DeleteTuple(
            rule->collection->Name(), time_, Clock::now(), tuple));
      }

      for (const LocalTupleId& dep_id : ids) {
        RETURN_IF_ERROR(lineagedb_client_->AddDerivedLineage(
            dep_id, rule_number, is_insert, physical_time,
            LocalTupleId{rule->collection->Name(), hash(tuple), time_}));
      }

      physical_time = Clock::now();
    };

    for (const auto& t : ts) {
      detail::UpdateCollection(rule->collection, t, hash(t), time_, RuleTag());
    }

    return common::Status::OK;
  }

  // The logical time of the fluent program. The logical time begins at 0 and
  // is ticked before every rule execution and before every round of receiving
  // tuples. More concretely, grep for `time_++`.
  int time_ = 0;

  // See `FluentBuilder`.
  const std::string name_;
  const std::size_t id_;
  std::tuple<std::unique_ptr<Collections>...> collections_;
  BootstrapRulesTuple bootstrap_rules_;
  std::unique_ptr<TimestampWrapper> logical_time_wrapper_;
  std::unique_ptr<NetworkState> network_state_;
  collections::Stdin* const stdin_;
  std::vector<collections::Periodic<Clock>*> periodics_;
  std::unique_ptr<LineageDbClient<Hash, ToSql, Clock>> lineagedb_client_;

  // A collection of rules (lhs, type, rhs) where
  //
  //   - `lhs` is a pointer to a collection,
  //   - `type` is an instance of one of the structs below, and
  //   - `rhs` is a relational algebra expression.
  //
  // See the class comment at the top of the file or see rule_tags.h for more
  // information.
  RulesTuple rules_;

  // When a FluentExecutor runs its Receive method, it has to worry about three
  // types of events triggering:
  //
  //   1. a fluent program can receive a message (via a channel) from a
  //      different fluent node,
  //   2. a fluent program can read a message from stdin, or
  //   3. a Periodic in a fluent program can trigger.
  //
  // To simultaneously wait for the first two events, we perform a zmq::poll.
  // To wait for the last type of event, we have to set the timeout to
  // zmq::poll just right. Here's how that works.
  //
  // We maintain a priority queue `timeout_queue_` of PeriodicTimeout structs:
  // one for each Periodic in `periodics_`. Each PeriodicTimeout records the
  // time at which it should be triggered and the associated Periodic. These
  // PeriodicTimeouts are ordered by increasing timeout using the
  // PeriodicTimeoutCompare struct below.
  //
  // For example, imagine we have 3 periodics:
  //
  //   - x with a period of 2,
  //   - y with a period of 4, and
  //   - z with a period of 16.
  //
  // Here is the contents of `timeout_queue_` at time 0.
  //
  //           +--------------+--------------+--------------+
  //   time  0 | timeout:  2  | timeout:  4  | timeout:  16 |
  //           | periodic: &x | periodic: &y | periodic: &z |
  //           +--------------+--------------+--------------+
  //
  // Then, we set our poll timeout to wake up just in time for the first
  // timeout. In our example, our poll timeout would be 2. Every time, the poll
  // is triggered, we iterate through the PeriodicTimeouts that should be
  // triggered, trigger them, update their timeout, and then push them back
  // onto the queue. Here's what the state of the queue looks like over time:
  //
  //           +--------------+--------------+--------------+
  //   time  2 | timeout:  4  | timeout:  4  | timeout:  16 |
  //           | periodic: &x | periodic: &y | periodic: &z |
  //           +--------------+--------------+--------------+
  //           +--------------+--------------+--------------+
  //   time  4 | timeout:  6  | timeout:  8  | timeout:  16 |
  //           | periodic: &x | periodic: &y | periodic: &z |
  //           +--------------+--------------+--------------+
  //           +--------------+--------------+--------------+
  //   time  6 | timeout:  8  | timeout:  8  | timeout:  16 |
  //           | periodic: &x | periodic: &y | periodic: &z |
  //           +--------------+--------------+--------------+
  //   ...
  //           +--------------+--------------+--------------+
  //   time 16 | timeout:  18 | timeout:  20 | timeout:  32 |
  //           | periodic: &x | periodic: &y | periodic: &z |
  //           +--------------+--------------+--------------+
  struct PeriodicTimeout {
    Time timeout;
    collections::Periodic<Clock>* periodic;
  };

  // See `PeriodicTimeout`.
  struct PeriodicTimeoutCompare {
    bool operator()(const PeriodicTimeout& lhs, const PeriodicTimeout& rhs) {
      return lhs.timeout < rhs.timeout;
    }
  };

  // See `PeriodicTimeout`.
  std::priority_queue<PeriodicTimeout, std::vector<PeriodicTimeout>,
                      PeriodicTimeoutCompare>
      timeout_queue_;

  FRIEND_TEST(FluentExecutor, SimpleCommunication);
};

}  // namespace fluent

#endif  // FLUENT_FLUENT_EXECUTOR_H_
