#include <cstdint>

#include <chrono>
#include <map>
#include <random>
#include <vector>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/rand_util.h"
#include "common/string_util.h"
#include "examples/cassandra/api.h"
#include "examples/cassandra/workloads.h"
#include "fluent/fluent.h"

namespace lra = fluent::ra::logical;
namespace ldb = fluent::lineagedb;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 11) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_host> \\" << std::endl               //
              << "  <db_port> \\" << std::endl               //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <server_address> \\" << std::endl        //
              << "  <client_address> \\" << std::endl
              << "  <msg_rate_ms> \\" << std::endl  //
              << "  <workload> \\" << std::endl     //
              << "  <num_keys> \\" << std::endl     //
        ;
    return 1;
  }

  // Command line arguments.
  const std::string db_host = argv[1];
  const int db_port = std::stoi(argv[2]);
  const std::string db_user = argv[3];
  const std::string db_password = argv[4];
  const std::string db_dbname = argv[5];
  const std::string server_address = argv[6];
  const std::string client_address = argv[7];
  const int msg_rate_ms = std::stoi(argv[8]);
  const Workload workload = StringToWorkload(argv[9]);
  const int num_keys = std::stoi(argv[10]);

  LOG(INFO) << "db_host        = " << db_host;
  LOG(INFO) << "db_port        = " << db_port;
  LOG(INFO) << "db_user        = " << db_user;
  LOG(INFO) << "db_password    = " << db_password;
  LOG(INFO) << "db_dbname      = " << db_dbname;
  LOG(INFO) << "server_address = " << server_address;
  LOG(INFO) << "client_address = " << client_address;
  LOG(INFO) << "msg_rate_ms    = " << msg_rate_ms;
  LOG(INFO) << "workload       = " << WorkloadToString(workload);
  LOG(INFO) << "num_keys       = " << num_keys;

  // Workload distribution_.
  std::random_device random_device;
  std::mt19937 engine(random_device());
  std::discrete_distribution<int> workload_distribution =
      WorkloadToDistribution(workload, num_keys);

  // Random id generator.
  fluent::RandomIdGenerator id_gen;

  // ZeroMQ context.
  zmq::context_t context(1);

  // Lineage database configuration.
  ldb::ConnectionConfig config;
  config.host = db_host;
  config.port = db_port;
  config.user = db_user;
  config.password = db_password;
  config.dbname = db_dbname;

  const std::string name =
      "cassandra_benchmark_client_getter_" + fluent::RandomAlphanum(10);
  auto fb =
      fluent::fluent<ldb::PqxxClient>(name, client_address, &context, config)
          .ConsumeValueOrDie()
          .periodic("p", std::chrono::milliseconds(msg_rate_ms));
  auto f =
      AddKvsApi(std::move(fb))
          .RegisterRules([&](auto& p, auto& set_request, auto& set_response,
                             auto& get_request, auto& get_response) {
            UNUSED(set_request);
            UNUSED(set_response);
            UNUSED(get_response);

            using namespace fluent::infix;

            auto send_get = get_request <=
                            (lra::make_collection(&p) |
                             lra::map([&](const auto&) -> get_request_tuple {
                               const std::int64_t id = id_gen.Generate();
                               const std::string key = std::to_string(
                                   workload_distribution(engine));
                               VLOG(1) << "GET " << key;
                               return {server_address, client_address, id, key};
                             }));

            return std::make_tuple(send_get);
          })
          .ConsumeValueOrDie();

  std::cout << "Getter running for 25 seconds." << std::endl;

  using namespace std::chrono;
  nanoseconds duration = seconds(25);
  time_point<system_clock> start = system_clock::now();
  time_point<system_clock> stop = start + duration;

  while (system_clock::now() < stop) {
    CHECK_EQ(f.Tick(), fluent::Status::OK);
    CHECK_EQ(f.Receive(), fluent::Status::OK);
  }

  std::cout << "Getter complete! Now run 'pg_dump " << db_dbname
            << " > outfile'" << std::endl;
}
