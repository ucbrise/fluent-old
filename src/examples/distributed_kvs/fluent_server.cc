#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "glog/logging.h"
#include "zmq.hpp"

#include "common/macros.h"
#include "common/rand_util.h"
#include "examples/distributed_kvs/client.h"
#include "examples/distributed_kvs/fluent_api.h"
#include "fluent/fluent.h"

namespace lra = fluent::ra::logical;
namespace ldb = fluent::lineagedb;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 9) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <kvs_address> \\" << std::endl           //
              << "  <replica_index> \\" << std::endl         //
              << "  <replica_address1> \\" << std::endl      //
              << "  <replica_address2> \\" << std::endl      //
              << "  <replica_address3> \\" << std::endl;
    return 1;
  }

  // Command line arguments.
  const std::string db_user = argv[1];
  const std::string db_password = argv[2];
  const std::string db_dbname = argv[3];
  const std::string kvs_address = argv[4];
  const int replica_index = std::stoi(argv[5]);
  std::vector<std::string> replica_addresses;
  replica_addresses.push_back(argv[6]);
  replica_addresses.push_back(argv[7]);
  replica_addresses.push_back(argv[8]);
  CHECK_GE(replica_index, 0);
  CHECK_LE(replica_index, 2);
  const std::string replica_address = replica_addresses[replica_index];
  replica_addresses.erase(replica_addresses.begin() + replica_index);

  // Initial vector clock value.
  std::vector<int> vc_vector = {0, 0, 0};
  std::tuple<std::vector<int>> vc_tuple = {vc_vector};
  std::set<std::tuple<std::vector<int>>> initial_vector_clock = {vc_tuple};

  // ZeroMQ socket.
  zmq::context_t context(1);

  // Lineage database configuration.
  ldb::ConnectionConfig config;
  config.host = "localhost";
  config.port = 5432;
  config.user = db_user;
  config.password = db_password;
  config.dbname = db_dbname;

  // GRPC client.
  std::shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(kvs_address, grpc::InsecureChannelCredentials());
  KeyValueServiceClient grpc_client(channel);

  const std::string name = "dkvs_server_" + std::to_string(replica_index);
  auto fb_or =
      fluent::fluent<ldb::PqxxClient>(name, replica_address, &context, config);
  auto fb = fb_or.ConsumeValueOrDie();

  auto with_collections =
      AddKvsApi(std::move(fb))
          .logical_time()
          .table<std::vector<int>>("vector_clock", {{"clock"}})
          .periodic("gossip_period", std::chrono::seconds(5))
          .channel<std::string, std::vector<int>>("vector_clock_merge",
                                                  {{"addr", "clock"}});

  auto with_bootstrap_rule =
      std::move(with_collections)
          .RegisterBootstrapRules([&initial_vector_clock](
              auto& logical_time, auto& set_request, auto& set_response,
              auto& get_request, auto& get_response, auto& vector_clock,
              auto& gossip_period, auto& vector_clock_merge) {
            UNUSED(logical_time);
            UNUSED(set_request);
            UNUSED(set_response);
            UNUSED(get_request);
            UNUSED(get_response);
            UNUSED(gossip_period);
            UNUSED(vector_clock_merge);

            using namespace fluent::infix;
            auto vc_iterable = lra::make_iterable(&initial_vector_clock);
            auto rule = (vector_clock <= vc_iterable);
            return std::make_tuple(rule);
          });

  auto with_rules_or =
      std::move(with_bootstrap_rule)
          .RegisterRules([&](auto& logical_time, auto& set_request,
                             auto& set_response, auto& get_request,
                             auto& get_response, auto& vector_clock,
                             auto& gossip_period, auto& vector_clock_merge) {
            using namespace fluent::infix;

            auto set = set_response <=
                       (lra::make_collection(&set_request) |
                        lra::map([&grpc_client](const set_request_tuple& t) {
                          const std::string& src_addr = std::get<1>(t);
                          const std::int64_t id = std::get<2>(t);
                          const std::string& key = std::get<3>(t);
                          const std::int32_t value = std::get<4>(t);
                          const std::int64_t timestamp = std::get<5>(t);
                          grpc_client.Set(key, value, id, timestamp);
                          return set_response_tuple(src_addr, id);
                        }));

            auto get =
                get_response <=
                (lra::make_collection(&get_request) |
                 lra::map([&grpc_client](const get_request_tuple& t) {
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t id = std::get<2>(t);
                   const std::string& key = std::get<3>(t);

                   const std::tuple<std::int32_t, std::int64_t> reply =
                       grpc_client.Get(key);
                   const std::int32_t value = std::get<0>(reply);
                   const std::int64_t reply_id = std::get<1>(reply);
                   return get_response_tuple(src_addr, id, value, reply_id);
                 }));

            auto gossip =
                vector_clock_merge <=
                (lra::make_cross(lra::make_collection(&gossip_period),
                                 lra::make_collection(&vector_clock)) |
                 lra::map([&](const auto& t) {
                   std::vector<int> clock = std::get<2>(t);
                   clock[replica_index] = logical_time.Get();
                   const std::string& replica_address =
                       replica_addresses[fluent::common::RandInt(0, 2)];
                   return std::make_tuple(replica_address, clock);
                 }));

            auto delete_vector_clock_gossip = vector_clock -=
                (lra::make_cross(lra::make_collection(&vector_clock_merge),
                                 lra::make_collection(&vector_clock)) |
                 lra::project<2>());

            auto update_vector_clock_gossip = vector_clock +=
                (lra::make_cross(lra::make_collection(&vector_clock_merge),
                                 lra::make_collection(&vector_clock)) |
                 lra::map([&](const auto& t) {
                   const std::vector<int>& remote_clock = std::get<1>(t);
                   std::vector<int> clock = std::get<2>(t);
                   CHECK_EQ(remote_clock.size(), clock.size());
                   for (std::size_t i = 0; i < clock.size(); ++i) {
                     clock[i] = std::max(remote_clock[i], clock[i]);
                   }
                   clock[replica_index] = logical_time.Get();
                   return std::make_tuple(clock);
                 }));

            return std::make_tuple(set, get, gossip, delete_vector_clock_gossip,
                                   update_vector_clock_gossip);
          });
  auto with_rules = with_rules_or.ConsumeValueOrDie();

  // TODO(mwhittaker): Remove this hack.
  std::this_thread::sleep_for(std::chrono::seconds(1));
  fluent::common::Status status = with_rules.RegisterBlackBoxLineage<2, 3>(
      [&replica_index](const std::string& time_inserted, const std::string& key,
                       const std::string& value, const std::string& id) {
        UNUSED(value);
        UNUSED(id);

        int primary = replica_index;
        int backup_a = 0;
        int backup_b = 0;
        if (replica_index == 0) {
          backup_a = 1;
          backup_b = 2;
        } else if (replica_index == 1) {
          backup_a = 0;
          backup_b = 2;
        } else {
          CHECK_EQ(replica_index, 2);
          backup_a = 0;
          backup_b = 1;
        }

        const std::string query = R"(
          WITH

          vector_clock_{p}(clock) AS (
              SELECT clock[1:{p}] || {time_inserted} || clock[{pplustwo}:3]
              FROM {server_name}_{p}_vector_clock
              WHERE time_inserted <= {time_inserted}
              ORDER BY time_inserted DESC
              LIMIT 1
          ),

          lineage_{a}(node, collection, hash, time_inserted, physical_time) AS (
            SELECT CAST('{server_name}_{a}' AS text),
                   CAST('set_request' AS text),
                   hash,
                   time_inserted,
                   physical_time_inserted
            FROM {server_name}_{a}_set_request SR
            WHERE key = {key} AND NOT EXISTS(
                SELECT *
                FROM (SELECT    VC{a}.clock
                      FROM      {server_name}_{a}_vector_clock VC{a},
                                vector_clock_{p} VC{p}
                       WHERE    VC{a}.time_inserted <= SR.time_inserted
                       ORDER BY VC{a}.time_inserted DESC
                       LIMIT 1) VC{a},
                      vector_clock_{p} VC{p}
                WHERE NOT (VectorClockGe(VC{p}.clock, VC{a}.clock) OR
                           VectorClockConcurrent(VC{p}.clock, VC{a}.clock))

            )
          ),

          lineage_{b}(node, collection, hash, time_inserted, physical_time) AS (
            SELECT CAST('{server_name}_{b}' AS text),
                   CAST('set_request' AS text),
                   hash,
                   time_inserted,
                   physical_time_inserted
            FROM {server_name}_{b}_set_request SR
            WHERE key = {key} AND NOT EXISTS(
                SELECT *
                FROM (SELECT    VC{b}.clock
                      FROM      {server_name}_{b}_vector_clock VC{b},
                                vector_clock_{p} VC{p}
                       WHERE    VC{b}.time_inserted <= SR.time_inserted
                       ORDER BY VC{b}.time_inserted DESC
                       LIMIT 1) VC{b},
                      vector_clock_{p} VC{p}
                WHERE NOT (VectorClockGe(VC{p}.clock, VC{b}.clock) OR
                           VectorClockConcurrent(VC{p}.clock, VC{b}.clock))

            )
          ),

          lineage_{p}(node, collection, hash, time_inserted, physical_time) AS (
            SELECT CAST('{server_name}_{p}' AS text),
                   CAST('set_request' AS text),
                   hash,
                   time_inserted,
                   physical_time_inserted
            FROM {server_name}_{p}_set_request
            WHERE time_inserted <= {time_inserted} AND key = {key}
            ORDER BY time_inserted DESC
            LIMIT 1
          )

          SELECT * FROM lineage_{a} UNION
          SELECT * FROM lineage_{b} UNION
          SELECT * FROM lineage_{p}
        )";
        return fmt::format(
            query, fmt::arg("server_name", "dkvs_server"), fmt::arg("key", key),
            fmt::arg("time_inserted", time_inserted), fmt::arg("p", primary),
            fmt::arg("pplustwo", primary + 2), fmt::arg("a", backup_a),
            fmt::arg("b", backup_b));
      });

  CHECK_EQ(status, fluent::common::Status::OK);
  CHECK_EQ(with_rules.Run(), fluent::common::Status::OK);

  return 0;
}
