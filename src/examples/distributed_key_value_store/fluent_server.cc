#include <string>
#include <vector>

#include "glog/logging.h"
#include "zmq.hpp"

#include "common/macros.h"
#include "common/rand_util.h"
#include "examples/distributed_key_value_store/client.h"
#include "examples/distributed_key_value_store/fluent_api.h"
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

  const std::string name =
      "distributed_kvs_server_" + fluent::RandomAlphanum(10);
  auto fb_or =
      fluent::fluent<ldb::PqxxClient>(name, replica_address, &context, config);
  auto fb = fb_or.ConsumeValueOrDie();

  auto with_collections =
      AddKvsApi(std::move(fb))
          .table<std::vector<int>>("vector_clock", {{"clock"}})
          .periodic("gossip_period", std::chrono::seconds(5))
          .channel<std::string, std::vector<int>>("vector_clock_merge",
                                                  {{"addr", "clock"}});

  auto with_bootstrap_rule =
      std::move(with_collections)
          .RegisterBootstrapRules([&initial_vector_clock](
              auto& set_request, auto& set_response, auto& get_request,
              auto& get_response, auto& vector_clock, auto& gossip_period,
              auto& vector_clock_merge) {
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
          .RegisterRules([&](auto& set_request, auto& set_response,
                             auto& get_request, auto& get_response,
                             auto& vector_clock, auto& gossip_period,
                             auto& vector_clock_merge) {
            using namespace fluent::infix;

            auto set = set_response <=
                       (lra::make_collection(&set_request) |
                        lra::map([&grpc_client](const set_request_tuple& t) {
                          const std::string& src_addr = std::get<1>(t);
                          const std::int64_t id = std::get<2>(t);
                          const std::string& key = std::get<3>(t);
                          const std::int64_t value = std::get<4>(t);
                          grpc_client.Set(key, value);
                          return set_response_tuple(src_addr, id);
                        }));

            auto get = get_response <=
                       (lra::make_collection(&get_request) |
                        lra::map([&grpc_client](const get_request_tuple& t) {
                          const std::string& src_addr = std::get<1>(t);
                          const std::int64_t id = std::get<2>(t);
                          const std::string& key = std::get<3>(t);
                          const std::int64_t value = grpc_client.Get(key);
                          return get_response_tuple(src_addr, id, value);
                        }));

            auto gossip =
                vector_clock_merge <=
                (lra::make_cross(lra::make_collection(&gossip_period),
                                 lra::make_collection(&vector_clock)) |
                 lra::map([&replica_addresses](const auto& t) {
                   std::vector<int> clock = std::get<2>(t);
                   const std::string& replica_address =
                       replica_addresses[fluent::RandInt(0, 2)];
                   return std::make_tuple(replica_address, clock);
                 }));

            return std::make_tuple(set, get, gossip);
          });
  auto with_rules = with_rules_or.ConsumeValueOrDie();
  CHECK_EQ(with_rules.Run(), fluent::Status::OK);

  return 0;
}
