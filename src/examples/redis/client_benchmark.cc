#include <chrono>
#include <cstdint>
#include <map>
#include <tuple>
#include <vector>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/mock_pickler.h"
#include "common/rand_util.h"
#include "common/status.h"
#include "common/string_util.h"
#include "examples/redis/redis.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/noop_client.h"
#include "ra/logical/all.h"

namespace lra = fluent::ra::logical;
namespace ldb = fluent::lineagedb;

using fluent::common::Hash;
using fluent::ldb::ToSql;
using fluent::ldb::NoopClient;
using fluent::common::MockPickler;

using set_req_tuple = std::tuple<std::string, std::string, std::int64_t,
                                 std::string, std::string>;
using set_resp_tuple = std::tuple<std::string, std::int64_t, bool>;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 4) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <server_address> \\" << std::endl        //
              << "  <client_address> \\" << std::endl        //
              << "  <nickname> \\" << std::endl              //
        ;
    return 1;
  }

  const std::string server_addr = argv[1];
  const std::string client_addr = argv[2];
  const std::string nickname = argv[3];

  const std::string name = "redis_client_benchmark_" + nickname;
  fluent::common::RandomIdGenerator id_gen;
  zmq::context_t context(1);
  fluent::lineagedb::ConnectionConfig conf;
  std::set<std::tuple<>> dummy = {std::tuple<>()};
  set_req_tuple t(server_addr, client_addr, 0, "k", "1");

  using std::string;
  auto f =
      fluent::fluent<NoopClient, Hash, ToSql, MockPickler>(name, client_addr,
                                                           &context, conf)
          .ConsumeValueOrDie()
          .channel<string, string, std::int64_t, string, string>(
              "set_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
          .channel<string, std::int64_t, bool>(  //
              "set_response", {{"addr", "id", "success"}})
          .RegisterRules([&](auto& set_req, auto&) {
            using namespace fluent::infix;

            auto send_set =
                set_req <= (lra::make_iterable(&dummy) |
                            lra::map([&](const auto&) -> set_req_tuple {
                              const std::int64_t id = id_gen.Generate();
                              std::get<2>(t) = id;
                              return t;
                            }));

            return std::make_tuple(send_set);
          })
          .ConsumeValueOrDie();

  using namespace std::chrono;
  nanoseconds duration = seconds(5);
  time_point<system_clock> start = system_clock::now();
  time_point<system_clock> stop = start + duration;
  int count = 1;

  for (count = 1; system_clock::now() < stop; ++count) {
    CHECK_EQ(f.Tick(), fluent::common::Status::OK);
    CHECK_EQ(f.Receive(), fluent::common::Status::OK);
  }

  nanoseconds elapsed = system_clock::now() - start;
  double seconds = elapsed.count() / 1e9;
  double frequency = static_cast<double>(count - 1) / seconds;
  std::cout << fmt::format("{},{},{}", count - 1, seconds, frequency)
            << std::endl;
}
