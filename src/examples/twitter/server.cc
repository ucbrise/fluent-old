#include <cstdint>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/status.h"
#include "examples/twitter/twitter.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/pqxx_client.h"
#include "ra/all.h"

namespace ra = fluent::ra;
namespace ldb = fluent::lineagedb;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 6) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <redis_address> \\" << std::endl         //
              << "  <server_address> \\" << std::endl;
    return 1;
  }

  const std::string redis_address = argv[4];
  const std::string server_address = argv[5];
  int tweet_id = 0;

  zmq::context_t context(1);
  ldb::ConnectionConfig config{"localhost", 5432, argv[1], argv[2], argv[3]};
  auto f =  //
      fluent::fluent<ldb::PqxxClient>("twitter_server", server_address,
                                      &context, config)
          .ConsumeValueOrDie()
          .channel<std::string, std::string, std::int64_t, std::string,
                   std::string>(
              "set_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
          .channel<std::string, std::string, std::int64_t, std::string>(
              "get_request", {{"dst_addr", "src_addr", "id", "key"}})
          .channel<std::string, std::int64_t, std::string>(
              "get_response", {{"addr", "id", "value"}})
          .scratch<std::string, std::string, int>(  //
              "tweet_id_buf", {{"addr", "tweet", "tweet_id"}})
          .table<std::string>(  //
              "fetch_buf", {{"addr"}});
  fluent::Status status =
      AddTwitterApi(std::move(f))
          .RegisterRules([&](auto& set_request, auto& get_request,
                             auto& get_response, auto& tweet_id_buf,
                             auto& fetch_buf, auto& tweet_req, auto& tweet_resp,
                             auto& fetch_req, auto& fetch_resp, auto& update) {
            using namespace fluent::infix;
            using namespace std;

            auto gen_id =
                tweet_id_buf <=
                (tweet_req.Iterable()  //
                 | ra::map([&](const auto& t) -> tuple<string, string, int> {
                     tweet_id++;
                     return {get<1>(t), get<2>(t), tweet_id};
                   }));

            auto return_id =
                tweet_resp <= (tweet_id_buf.Iterable() | ra::project<0, 2>());

            auto send_tweet =
                set_request <=
                (tweet_id_buf.Iterable()  //
                 |
                 ra::map([&](const auto& t)
                             -> tuple<string, string, int64_t, string, string> {
                   const int tweet_id = get<2>(t);
                   const string& tweet = get<1>(t);
                   return {redis_address, server_address, 0,
                           to_string(tweet_id), tweet};

                 }));

            auto send_update =
                set_request <=
                (update.Iterable() |
                 ra::map([&](const auto& t)
                             -> tuple<string, string, int64_t, string, string> {
                   const int tweet_id = get<1>(t);
                   const string& tweet = get<2>(t);
                   return {redis_address, server_address, 0,
                           to_string(tweet_id), tweet};

                 }));

            auto buffer_fetch =
                fetch_buf <= (fetch_req.Iterable() | ra::project<1>());

            auto send_get =
                get_request <=
                (fetch_req.Iterable()  //
                 | ra::map([&](const auto& t)
                               -> tuple<string, string, int64_t, string> {
                     const int tweet_id = get<2>(t);
                     return {redis_address, server_address, 0,
                             to_string(tweet_id)};
                   }));

            auto send_fetch_resp =
                fetch_resp <=
                (ra::make_cross(fetch_buf.Iterable(),
                                get_response.Iterable())  //
                 | ra::map([](const auto& t) -> tuple<string, string> {
                     const std::string& addr = get<0>(t);
                     const std::string& tweet = get<3>(t);
                     return {addr, tweet};
                   }));

            return std::make_tuple(gen_id, return_id, send_tweet, send_update,
                                   buffer_fetch, send_get, send_fetch_resp);
          })
          .ConsumeValueOrDie()
          .Run();
  CHECK_EQ(fluent::Status::OK, status);
}
