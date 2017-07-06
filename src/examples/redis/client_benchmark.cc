#include <chrono>
#include <cstdint>
#include <map>
#include <vector>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

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

double time_s() {
  unsigned long ms = std::chrono::system_clock::now().time_since_epoch() / std::chrono::microseconds(1);
  return (double)ms / 1e6;
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 4) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <server_address> \\" << std::endl        //
              << "  <client_address> \\" << std::endl        //
              << "  <name> \\" << std::endl                  //
              ;
    return 1;
  }

  const std::string server_address = argv[1];
  const std::string client_address = argv[2];
  const std::string name = argv[3];
  fluent::RandomIdGenerator id_gen;

  int count = 1;
  const double t = 5; // s
  double t0;
  double t_end;
  double t_elapsed;
  bool continueTest = true;

  std::vector<std::tuple<std::string, std::string, std::int64_t, std::string, std::string>> init_vector =
    { std::make_tuple(server_address, client_address, id_gen.Generate(), "simple_loop:count", "0") };

  zmq::context_t context(1);
  fluent::lineagedb::ConnectionConfig config;

  auto fb = fluent::fluent<fluent::lineagedb::NoopClient>("redis_client_" + name,
                                            client_address, &context, config)
                .ConsumeValueOrDie()
                .stdin()
                .stdout()
                .scratch<std::vector<std::string>>("split", {{"parts"}});
  AddRedisApi(std::move(fb))
      .RegisterBootstrapRules([&](auto& stdin, auto& stdout, auto& split,
                         auto& set_req, auto& set_resp, auto& append_req,
                         auto& append_resp, auto& get_req, auto& get_resp) {
        using namespace fluent::infix;

        (void)stdin;
        (void)stdout;
        (void)split;
        (void)set_resp;
        (void)append_req;
        (void)append_resp;
        (void)get_req;
        (void)get_resp;

        auto initial_set =
            set_req <= (lra::make_iterable(&init_vector));


        return std::make_tuple(initial_set);

      })
      .RegisterRules([&](auto& stdin, auto& stdout, auto& split,
                         auto& set_req, auto& set_resp, auto& append_req,
                         auto& append_resp, auto& get_req, auto& get_resp) {
        using namespace fluent::infix;

        (void)append_req;
        (void)append_resp;
        (void)get_req;
        (void)get_resp;

        auto buffer_stdin =
            split <= (lra::make_collection(&stdin) |
                      lra::map([](const std::tuple<std::string>& s)
                                   -> std::tuple<std::vector<std::string>> {
                        return {fluent::Split(std::get<0>(s))};
                      }));

        auto set_request =
            set_req <=
            (lra::make_collection(&split) |
             lra::filter([](
                 const std::tuple<std::vector<std::string>>& parts_tuple) {
               const std::vector<std::string>& parts = std::get<0>(parts_tuple);
               return parts.size() == 1 && parts[0] == "START";
             }) |
             lra::map(
                 [&count, &t0, &t, &t_end, &server_address, &client_address, &id_gen](const std::tuple<std::vector<std::string>>&)
                     -> std::tuple<std::string, std::string, std::int64_t,
                                   std::string, std::string> {
                   t0 = time_s();
                   t_end = t0 + t;

                   return {server_address, client_address, id_gen.Generate(),
                           "simple_loop:count", std::to_string(count++)};
                 }));

        auto set_response =
            set_req <= (lra::make_collection(&set_resp)
            | lra::filter([&count, &t0, &t_end, &t_elapsed, &continueTest](
                 const std::tuple<std::string, std::int64_t, bool>&) {
               t_elapsed = time_s() - t0;
               continueTest = time_s() < t_end;
               double actual_freq = (double)(count - 1) / t_elapsed;

               if (!continueTest && (t0 != 0))
               {
                  LOG(INFO) << "Fromt t0 = " << t0 << " , t_end = " << t_end << " , t_elapsed = " << t_elapsed << std::endl;

                  LOG(INFO) << "Sent " << (count - 1) << " commands in " << t_elapsed << "s, "
                    << "that's " << actual_freq << " commands/s." << std::endl;
               }

               return continueTest;
             })
            | lra::map([&count, &server_address, &client_address, &id_gen](const std::tuple<std::string, std::int64_t, bool>&)
              -> std::tuple<std::string, std::string, std::int64_t,
                                   std::string, std::string> {
                return {server_address, client_address, id_gen.Generate(),
                           "simple_loop:count", std::to_string(count++)};
            }));

        return std::make_tuple(buffer_stdin, set_request, set_response);
      })
      .ConsumeValueOrDie()
      .Run();
}
