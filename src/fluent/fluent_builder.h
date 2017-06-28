#ifndef FLUENT_FLUENT_BUILDER_H_
#define FLUENT_FLUENT_BUILDER_H_

#include <cstddef>

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "zmq.hpp"

#include "collections/all.h"
#include "collections/collection.h"
#include "common/cereal_pickler.h"
#include "common/hash_util.h"
#include "common/macros.h"
#include "common/static_assert.h"
#include "common/status.h"
#include "common/status_macros.h"
#include "common/status_or.h"
#include "common/string_util.h"
#include "common/type_list.h"
#include "common/type_traits.h"
#include "fluent/fluent_executor.h"
#include "fluent/network_state.h"
#include "fluent/rule.h"
#include "fluent/timestamp_wrapper.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/to_sql.h"
#include "ra/logical/logical_ra.h"
#include "zmq_util/socket_cache.h"

namespace fluent {
namespace detail {

template <typename Rules>
struct ValidateRules;

template <typename... Collections, typename... RuleTags, typename... Ras>
struct ValidateRules<TypeList<Rule<Collections, RuleTags, Ras>...>> {
  using all_collections = All<std::is_base_of<Collection, Collections>...>;
  static_assert(StaticAssert<all_collections>::value, "");
  using all_rule_tags = All<std::is_base_of<RuleTag, RuleTags>...>;
  static_assert(StaticAssert<all_rule_tags>::value, "");
  using all_ras = All<std::is_base_of<ra::logical::LogicalRa, Ras>...>;
  static_assert(StaticAssert<all_ras>::value, "");
  static constexpr bool value = true;
};

}  // namespace detail

// See below.
template <
    typename Collections, typename BootstrapRules, bool UseTimestampWrapper,
    template <template <typename> class Hash, template <typename> class ToSql,
              typename Clock> class LineageDbClient,
    template <typename> class Hash = Hash,
    template <typename> class ToSql = lineagedb::ToSql,
    template <typename> class Pickler = CerealPickler,
    typename Clock = std::chrono::system_clock>
class FluentBuilder;

template <typename... Collections, typename... BootstrapCollections,
          typename... BootstrapRuleTags, typename... BootstrapRas,
          bool UseTimestampWrapper,
          template <template <typename> class Hash,
                    template <typename> class ToSql, typename Clock>
          class LineageDbClient,
          template <typename> class Hash, template <typename> class ToSql,
          template <typename> class Pickler, typename Clock>
class FluentBuilder<
    TypeList<Collections...>,
    TypeList<Rule<BootstrapCollections, BootstrapRuleTags, BootstrapRas>...>,
    UseTimestampWrapper, LineageDbClient, Hash, ToSql, Pickler, Clock> {
  // Types /////////////////////////////////////////////////////////////////////
  using BootstrapRules =
      TypeList<Rule<BootstrapCollections, BootstrapRuleTags, BootstrapRas>...>;
  using BootstrapRulesTuple = typename TypeListToTuple<BootstrapRules>::type;

  template <typename Collection>
  using WithCollection =
      FluentBuilder<TypeList<Collections..., Collection>, BootstrapRules,
                    UseTimestampWrapper, LineageDbClient, Hash, ToSql, Pickler,
                    Clock>;

  template <typename BootstrapRules_>
  using WithBootstrapRules =
      FluentBuilder<TypeList<Collections...>, BootstrapRules_,
                    UseTimestampWrapper, LineageDbClient, Hash, ToSql, Pickler,
                    Clock>;

  using WithTimestampWrapper =
      FluentBuilder<TypeList<Collections...>, BootstrapRules, true,
                    LineageDbClient, Hash, ToSql, Pickler, Clock>;

  // Static Asserts ////////////////////////////////////////////////////////////
  using all_collections = All<std::is_base_of<Collection, Collections>...>;
  static_assert(StaticAssert<all_collections>::value, "");
  static_assert(StaticAssert<detail::ValidateRules<BootstrapRules>>::value, "");

 public:
  DISALLOW_COPY_AND_ASSIGN(FluentBuilder);
  DEFAULT_MOVE_AND_ASSIGN(FluentBuilder);

  // Collections ///////////////////////////////////////////////////////////////
  // Create a table, scratch, channel, stdin, stdout, or periodic. Note the
  // `&&` at the end of each declaration. This means that these methods can
  // only be invoked on an rvalue-reference, which is necessary since the
  // methods move their contents.
  template <typename... Us>
  WithCollection<Table<Us...>> table(
      const std::string& name,
      std::array<std::string, sizeof...(Us)> column_names) && {
    LOG(INFO) << "Adding table " << name << "(" << Join(column_names) << ").";
    return AddCollection(
        std::make_unique<Table<Us...>>(name, std::move(column_names)));
  }

  template <typename... Us>
  WithCollection<Scratch<Us...>> scratch(
      const std::string& name,
      std::array<std::string, sizeof...(Us)> column_names) && {
    LOG(INFO) << "Adding scratch " << name << "(" << Join(column_names) << ").";
    return AddCollection(
        std::make_unique<Scratch<Us...>>(name, std::move(column_names)));
  }

  template <typename... Us>
  WithCollection<Channel<Pickler, Us...>> channel(
      const std::string& name,
      std::array<std::string, sizeof...(Us)> column_names) && {
    LOG(INFO) << "Adding channel " << name << "(" << Join(column_names) << ").";
    auto c = std::make_unique<Channel<Pickler, Us...>>(
        id_, name, std::move(column_names), &network_state_->socket_cache);
    return AddCollection(std::move(c));
  }

  WithCollection<Stdin> stdin() && {
    LOG(INFO) << "Adding stdin.";
    auto stdin_ptr = std::make_unique<Stdin>();
    stdin_ = stdin_ptr.get();
    return AddCollection(std::move(stdin_ptr));
  }

  WithCollection<Stdout> stdout() && {
    LOG(INFO) << "Adding stdout.";
    return AddCollection(std::make_unique<Stdout>());
  }

  WithCollection<Periodic<Clock>> periodic(
      const std::string& name,
      const typename Periodic<Clock>::period& period) && {
    LOG(INFO) << "Adding Periodic named " << name << ".";
    auto p = std::make_unique<Periodic<Clock>>(name, period);
    periodics_.push_back(p.get());
    return AddCollection(std::move(p));
  }

  WithTimestampWrapper logical_time() && {
    LOG(INFO) << "Adding logical time.";
    return {std::move(name_),
            id_,
            std::move(collections_),
            std::move(bootstrap_rules_),
            std::make_unique<TimestampWrapper>(),
            std::move(network_state_),
            stdin_,
            std::move(periodics_),
            std::move(lineagedb_client_)};
  }

  // Rule Registration /////////////////////////////////////////////////////////
  // See `RegisterRules`
  template <typename F>
  auto RegisterBootstrapRules(const F& f) && {
    return RegisterBootstrapRulesDispatchTimestamp<UseTimestampWrapper>(f);
  }

  // Recall the example from above:
  //
  //   auto f = fluent(address)
  //     .table<int, char, float>("t1")
  //     .table<float, int>("t2")
  //     .scratch<int, int, float>("s")
  //     .channel<std::string, float, char>("c")
  //     .RegisterRules([](auto& t1, auto& t2, auto& s, auto& c) {
  //       return std::make_tuple(t2 <= t1.iterable | ra::project<2, 0>());
  //     });
  //
  // `f` is a function which takes in a reference to each of the collections
  // registered with the FluentBuilder in the order they are registered. It
  // outputs a tuple of rules where each rule is a pair of the form (lhs, rhs)
  // where
  //   - lhs is a pointer to a collection in the FluentBuilder, and
  //   - rhs is a relational algebra expression.
  // Note that `t <= ra` is syntactic sugar for `std::make_pair(t, ra)`
  // implemented by Collection's `<=` operator. `RegisterRules` will execute
  // `f` to generate the rules and use them to construct a `FluentExecutor`.
  template <typename F>
  auto RegisterRules(const F& f) && {
    return RegisterRulesDispatchTimestamp<UseTimestampWrapper>(f);
  }

 private:
  // Constructors //////////////////////////////////////////////////////////////
  static WARN_UNUSED StatusOr<FluentBuilder> Make(
      const std::string& name, const std::string& address,
      zmq::context_t* context,
      const lineagedb::ConnectionConfig& connection_config) {
    const std::size_t id = Hash<std::string>()(name);
    using Client = LineageDbClient<Hash, ToSql, Clock>;
    StatusOr<Client> client_or =
        Client::Make(name, id, address, connection_config);
    RETURN_IF_ERROR(client_or.status());
    return FluentBuilder(
        name, id, address, context,
        std::make_unique<Client>(client_or.ConsumeValueOrDie()));
  }

  // Constructs an empty FluentBuilder. Note that this constructor should only
  // be called when Collections and BootstrapRules are both empty. This private
  // constructor is used primarily by the `fluent` function down below.
  FluentBuilder(
      const std::string& name, std::size_t id, const std::string& address,
      zmq::context_t* context,
      std::unique_ptr<LineageDbClient<Hash, ToSql, Clock>> lineagedb_client)
      : name_(name),
        id_(id),
        network_state_(std::make_unique<NetworkState>(address, context)),
        stdin_(nullptr),
        lineagedb_client_(std::move(lineagedb_client)) {
    static_assert(
        sizeof...(Collections) == 0,
        "This constructor must only be called on empty FluentBuilders.");
    static_assert(
        TypeListLen<BootstrapRules>::value == 0,
        "This constructor must only be called on empty FluentBuilders.");
  }

  // Moves the guts of one FluentBuilder into another.
  FluentBuilder(
      std::string name, std::size_t id,
      std::tuple<std::unique_ptr<Collections>...> collections,
      BootstrapRulesTuple bootstrap_rules,
      std::unique_ptr<TimestampWrapper> logical_time_wrapper,
      std::unique_ptr<NetworkState> network_state, Stdin* stdin,
      std::vector<Periodic<Clock>*> periodics,
      std::unique_ptr<LineageDbClient<Hash, ToSql, Clock>> lineagedb_client)
      : name_(std::move(name)),
        id_(id),
        collections_(std::move(collections)),
        bootstrap_rules_(std::move(bootstrap_rules)),
        logical_time_wrapper_(std::move(logical_time_wrapper)),
        network_state_(std::move(network_state)),
        stdin_(stdin),
        periodics_(std::move(periodics)),
        lineagedb_client_(std::move(lineagedb_client)) {}

  // Collections ///////////////////////////////////////////////////////////////
  // Return a new FluentBuilder with `c` appended to `collections`.
  template <typename Collection>
  WithCollection<Collection> AddCollection(std::unique_ptr<Collection> c) {
    std::tuple<std::unique_ptr<Collections>..., std::unique_ptr<Collection>>
        collections = std::tuple_cat(std::move(collections_),
                                     std::make_tuple(std::move(c)));
    return {std::move(name_),
            id_,
            std::move(collections),
            std::move(bootstrap_rules_),
            std::move(logical_time_wrapper_),
            std::move(network_state_),
            stdin_,
            std::move(periodics_),
            std::move(lineagedb_client_)};
  }

  // Bootsrap Rules ////////////////////////////////////////////////////////////
  template <typename BootstrapRules, typename RetTuple, typename RetTypes>
  struct ValidateBootsrapRules {
    static_assert(TypeListLen<BootstrapRules>::value == 0,
                  "You are registering bootstrap rules with a FluentBuilder "
                  "that already has bootstrap rules registered with it. This "
                  "is disallowed.");
    static_assert(StaticAssert<IsTuple<RetTuple>>::value, "");
    static_assert(StaticAssert<detail::ValidateRules<RetTypes>>::value, "");
  };

  template <
      bool UseTimestampWrapper_, typename F,
      typename RetTuple = typename std::result_of<F(Collections&...)>::type,
      typename RetTypes = typename TupleToTypeList<RetTuple>::type>
  typename std::enable_if<!UseTimestampWrapper_,
                          WithBootstrapRules<RetTypes>>::type
  RegisterBootstrapRulesDispatchTimestamp(const F& f) {
    ValidateBootsrapRules<BootstrapRules, RetTuple, RetTypes>();
    return RegisterBootstrapRulesImplWithoutTimestampWrapper<F, RetTuple,
                                                             RetTypes>(
        f, std::make_index_sequence<sizeof...(Collections)>());
  }

  template <bool UseTimestampWrapper_, typename F,
            typename RetTuple = typename std::result_of<
                F(TimestampWrapper&, Collections&...)>::type,
            typename RetTypes = typename TupleToTypeList<RetTuple>::type>
  typename std::enable_if<UseTimestampWrapper_,
                          WithBootstrapRules<RetTypes>>::type
  RegisterBootstrapRulesDispatchTimestamp(const F& f) {
    ValidateBootsrapRules<BootstrapRules, RetTuple, RetTypes>();
    return RegisterBootstrapRulesImplWithTimestampWrapper<F, RetTuple,
                                                          RetTypes>(
        f, std::make_index_sequence<sizeof...(Collections)>());
  }

  template <typename F, typename RetTuple, typename RetTypes, std::size_t... Is>
  FluentBuilder<TypeList<Collections...>, RetTypes, UseTimestampWrapper,
                LineageDbClient, Hash, ToSql, Pickler, Clock>
  RegisterBootstrapRulesImplWithoutTimestampWrapper(
      const F& f, std::index_sequence<Is...>) {
    auto bootstrap_rules = f(*std::get<Is>(collections_)...);
    using correct_ret = std::is_same<decltype(bootstrap_rules), RetTuple>;
    static_assert(StaticAssert<correct_ret>::value, "");
    TupleIter(bootstrap_rules, [](const auto& rule) {
      LOG(INFO) << "Registering bootstrap rule: " << rule.ToDebugString();
    });
    return {std::move(name_),
            id_,
            std::move(collections_),
            std::move(bootstrap_rules),
            std::move(logical_time_wrapper_),
            std::move(network_state_),
            stdin_,
            std::move(periodics_),
            std::move(lineagedb_client_)};
  }

  template <typename F, typename RetTuple, typename RetTypes, std::size_t... Is>
  FluentBuilder<TypeList<Collections...>, RetTypes, UseTimestampWrapper,
                LineageDbClient, Hash, ToSql, Pickler, Clock>
  RegisterBootstrapRulesImplWithTimestampWrapper(const F& f,
                                                 std::index_sequence<Is...>) {
    auto bootstrap_rules =
        f(*logical_time_wrapper_, *std::get<Is>(collections_)...);
    using correct_ret = std::is_same<decltype(bootstrap_rules), RetTuple>;
    static_assert(StaticAssert<correct_ret>::value, "");
    TupleIter(bootstrap_rules, [](const auto& rule) {
      LOG(INFO) << "Registering bootstrap rule: " << rule.ToDebugString();
    });
    return {std::move(name_),
            id_,
            std::move(collections_),
            std::move(bootstrap_rules),
            std::move(logical_time_wrapper_),
            std::move(network_state_),
            stdin_,
            std::move(periodics_),
            std::move(lineagedb_client_)};
  }

  // Rules /////////////////////////////////////////////////////////////////////
  template <typename RetTuple, typename RetTypes>
  struct ValidateRules {
    static_assert(StaticAssert<IsTuple<RetTuple>>::value, "");
    static_assert(StaticAssert<detail::ValidateRules<RetTypes>>::value, "");
  };

  template <
      bool UseTimestampWrapper_, typename F,
      typename RetTuple = typename std::result_of<F(Collections&...)>::type,
      typename RetTypes = typename TupleToTypeList<RetTuple>::type>
  WARN_UNUSED typename std::enable_if<
      !UseTimestampWrapper_,
      StatusOr<
          FluentExecutor<TypeList<Collections...>, BootstrapRules, RetTypes,
                         LineageDbClient, Hash, ToSql, Pickler, Clock>>>::type
  RegisterRulesDispatchTimestamp(const F& f) {
    ValidateRules<RetTuple, RetTypes>();
    return RegisterRulesImplWithoutTimestampWrapper<F, RetTuple, RetTypes>(
        f, std::make_index_sequence<sizeof...(Collections)>());
  }

  template <bool UseTimestampWrapper_, typename F,
            typename RetTuple = typename std::result_of<
                F(TimestampWrapper&, Collections&...)>::type,
            typename RetTypes = typename TupleToTypeList<RetTuple>::type>
  WARN_UNUSED typename std::enable_if<
      UseTimestampWrapper_,
      StatusOr<
          FluentExecutor<TypeList<Collections...>, BootstrapRules, RetTypes,
                         LineageDbClient, Hash, ToSql, Pickler, Clock>>>::type
  RegisterRulesDispatchTimestamp(const F& f) {
    ValidateRules<RetTuple, RetTypes>();
    return RegisterRulesImplWithTimestampWrapper<F, RetTuple, RetTypes>(
        f, std::make_index_sequence<sizeof...(Collections)>());
  }

  template <typename F, typename RetTuple, typename RetTypes, std::size_t... Is,
            typename Executor = FluentExecutor<
                TypeList<Collections...>, BootstrapRules, RetTypes,
                LineageDbClient, Hash, ToSql, Pickler, Clock>>
  StatusOr<Executor> RegisterRulesImplWithoutTimestampWrapper(
      const F& f, std::index_sequence<Is...>) {
    auto rules = f(*std::get<Is>(collections_)...);
    using correct_ret = std::is_same<decltype(rules), RetTuple>;
    static_assert(StaticAssert<correct_ret>::value, "");
    TupleIter(rules, [](const auto& rule) {
      LOG(INFO) << "Registering rule: " << rule.ToDebugString();
    });
    return Executor::Make(
        std::move(name_), id_, std::move(collections_),
        std::move(bootstrap_rules_), std::move(logical_time_wrapper_),
        std::move(network_state_), stdin_, std::move(periodics_),
        std::move(lineagedb_client_), std::move(rules));
  }

  template <typename F, typename RetTuple, typename RetTypes, std::size_t... Is,
            typename Executor = FluentExecutor<
                TypeList<Collections...>, BootstrapRules, RetTypes,
                LineageDbClient, Hash, ToSql, Pickler, Clock>>
  StatusOr<Executor> RegisterRulesImplWithTimestampWrapper(
      const F& f, std::index_sequence<Is...>) {
    auto rules = f(*logical_time_wrapper_, *std::get<Is>(collections_)...);
    using correct_ret = std::is_same<decltype(rules), RetTuple>;
    static_assert(StaticAssert<correct_ret>::value, "");
    TupleIter(rules, [](const auto& rule) {
      LOG(INFO) << "Registering rule: " << rule.ToDebugString();
    });
    return Executor::Make(
        std::move(name_), id_, std::move(collections_),
        std::move(bootstrap_rules_), std::move(logical_time_wrapper_),
        std::move(network_state_), stdin_, std::move(periodics_),
        std::move(lineagedb_client_), std::move(rules));
  }

  // The name of the fluent program.
  const std::string name_;

  // The id of the fluent program.
  const std::size_t id_;

  // See class documentation above.
  //
  // TODO(mwhittaker): Right now, I made things unique_ptr because I was
  // paranoid that as data was being moved from one FluentBuilder to
  // another, pointers to fields would be invalidated. Maybe we don't need the
  // unique_ptr, but I'd need to think harder about it.
  std::tuple<std::unique_ptr<Collections>...> collections_;

  // See class documentation above.
  BootstrapRulesTuple bootstrap_rules_;

  // TODO(mwhittaker): Document.
  std::unique_ptr<TimestampWrapper> logical_time_wrapper_;

  // Each fluent program sends tuples to other fluent nodes over the network
  // and receives tuples from other fluent nodes over the network. This is the
  // networking state needed to perform that networking.
  std::unique_ptr<NetworkState> network_state_;

  // This pointer, if not null, points into the Stdin object inside of
  // `collections_`. If it is null, then there is no Stdin object in
  // `collections_`.
  Stdin* stdin_;

  // A vector of the Periodics inside of `collections_`, in no particular
  // order.
  std::vector<Periodic<Clock>*> periodics_;

  // A lineagedb client used to record history and lineage information.
  std::unique_ptr<LineageDbClient<Hash, ToSql, Clock>> lineagedb_client_;

  // Friends ///////////////////////////////////////////////////////////////////
  // All FluentBuilders are friends of one another.
  template <typename Collections_, typename BootstrapRules_,
            bool UseTimestampWrapper_,
            template <template <typename> class Hash_,
                      template <typename> class ToSql_, typename Clock_>
            class LineageDbClient_,
            template <typename> class Hash_, template <typename> class ToSql_,
            template <typename> class Pickler_, typename Clock_>
  friend class FluentBuilder;

  // The `fluent` function is used to construct an empty FluentBuilder.
  template <template <template <typename> class Hash_,
                      template <typename> class ToSql_, typename Clock_>
            class LineageDbClient_,
            template <typename> class Hash_, template <typename> class ToSql_,
            template <typename> class Pickler_, typename Clock_>
  friend StatusOr<FluentBuilder<TypeList<>, TypeList<>, false, LineageDbClient_,
                                Hash_, ToSql_, Pickler_, Clock_>>
  fluent(const std::string& name, const std::string& address,
         zmq::context_t* context,
         const lineagedb::ConnectionConfig& connection_config);
};

// Create an empty FluentBuilder.
template <template <template <typename> class Hash,
                    template <typename> class ToSql, typename Clock>
          class LineageDbClient,
          template <typename> class Hash = Hash,
          template <typename> class ToSql = lineagedb::ToSql,
          template <typename> class Pickler = CerealPickler,
          typename Clock = std::chrono::system_clock>
StatusOr<FluentBuilder<TypeList<>, TypeList<>, false, LineageDbClient, Hash,
                       ToSql, Pickler, Clock>>
fluent(const std::string& name, const std::string& address,
       zmq::context_t* context,
       const lineagedb::ConnectionConfig& connection_config) {
  return FluentBuilder<TypeList<>, TypeList<>, false, LineageDbClient, Hash,
                       ToSql, Pickler, Clock>::Make(name, address, context,
                                                    connection_config);
}

}  // namespace fluent

#endif  // FLUENT_FLUENT_BUILDER_H_
