#include <cstdint>

#include <string>

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

  if (argc != 7) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <server_address> \\" << std::endl        //
              << "  <client_address> \\" << std::endl        //
              << "  <name> \\" << std::endl;                 //
    return 1;
  }

  const std::string server_address = argv[4];
  const std::string client_address = argv[5];
  const std::string name = argv[6];

  zmq::context_t context(1);
  ldb::ConnectionConfig config{"localhost", 5432, argv[1], argv[2], argv[3]};
  auto f =  //
      fluent::fluent<ldb::PqxxClient>("twitter_client_" + name, client_address,
                                      &context, config)
          .ConsumeValueOrDie()
          .stdin()
          .stdout()
          .scratch<std::vector<std::string>>(  //
              "split", {{"parts"}})
          .table<std::string, std::string, int>(
              "forward_buf", {{"dst_addr", "tweeter", "tweet_id"}})
          .channel<std::string, std::string, std::string, int, std::string>(
              "forward",
              {{"dst_addr", "forwader", "tweeter", "tweet_id", "tweet"}});
  fluent::Status status =
      AddTwitterApi(std::move(f))  //
          .RegisterRules([&](auto& stdin, auto& stdout, auto& split,
                             auto& forward_buf, auto& forward, auto& tweet_req,
                             auto& tweet_resp, auto& fetch_req,
                             auto& fetch_resp, auto& update) {
            using namespace fluent::infix;
            using namespace std;

            auto buffer_stdin =
                split <=
                (stdin.Iterable()  //
                 | ra::map([](const tuple<string>& s) -> tuple<vector<string>> {
                     return {fluent::Split(get<0>(s))};
                   }));

            auto send_tweet =
                tweet_req <=
                (split.Iterable()  //
                 | ra::filter([](const tuple<vector<string>>& parts_tuple) {
                     const vector<string>& parts = get<0>(parts_tuple);
                     return parts.size() == 2 && parts[0] == "tweet";
                   })  //
                 | ra::map([&](const tuple<vector<string>>& parts_tuple)
                               -> tuple<string, string, string> {
                     const vector<string>& parts = get<0>(parts_tuple);
                     return {server_address, client_address, parts[1]};
                   }));

            auto recv_tweet =
                stdout <=
                (tweet_resp.Iterable()  //
                 | ra::project<1>()     //
                 | ra::map([](const tuple<int>& tweet_id) -> tuple<string> {
                     return {to_string(get<0>(tweet_id))};
                   }));

            auto update_tweet =
                update <=
                (split.Iterable()  //
                 | ra::filter([](const tuple<vector<string>>& parts_tuple) {
                     const vector<string>& parts = get<0>(parts_tuple);
                     return parts.size() == 3 && parts[0] == "update";
                   })  //
                 | ra::map([&](const tuple<vector<string>>& parts_tuple)
                               -> tuple<string, int, string> {
                     const vector<string>& parts = get<0>(parts_tuple);
                     return {server_address, stoi(parts[1]), parts[2]};
                   }));

            auto buffer_forward =
                forward_buf <=
                (split.Iterable()  //
                 | ra::filter([](const tuple<vector<string>>& parts_tuple) {
                     const vector<string>& parts = get<0>(parts_tuple);
                     return parts.size() == 4 && parts[0] == "forward";
                   })  //
                 | ra::map([&](const tuple<vector<string>>& parts_tuple)
                               -> tuple<string, string, int> {
                     const vector<string>& parts = get<0>(parts_tuple);
                     return {parts[1], parts[2], stoi(parts[3])};
                   }));

            auto fetch_request =
                fetch_req <=
                (split.Iterable()  //
                 | ra::filter([](const tuple<vector<string>>& parts_tuple) {
                     const vector<string>& parts = get<0>(parts_tuple);
                     return parts.size() == 4 && parts[0] == "forward";
                   })  //
                 | ra::map([&](const tuple<vector<string>>& parts_tuple)
                               -> tuple<string, string, int> {
                     const vector<string>& parts = get<0>(parts_tuple);
                     return {server_address, client_address, stoi(parts[3])};
                   }));

            auto fetch_response =
                forward <=
                (ra::make_cross(forward_buf.Iterable(),
                                fetch_resp.Iterable())  //
                 | ra::map([&](const auto& t)
                               -> tuple<string, string, string, int, string> {
                     const string& dst_addr = get<0>(t);
                     const string& forwarder = name;
                     const string& tweeter = get<1>(t);
                     const int tweet_id = get<2>(t);
                     const string& tweet = get<4>(t);
                     return {dst_addr, forwarder, tweeter, tweet_id, tweet};
                   }));

            auto print_forward =
                stdout <=
                (forward.Iterable()  //
                 | ra::map([](const auto& t) -> tuple<string> {
                     const string& forwarder = get<1>(t);
                     const string& tweeter = get<2>(t);
                     const int tweet_id = get<3>(t);
                     const string& tweet = get<4>(t);
                     return {fmt::format("Tweet {} forwarded from {}: {} -{}",
                                         tweet_id, forwarder, tweet, tweeter)};
                   }));

            return make_tuple(buffer_stdin, send_tweet, recv_tweet,
                              update_tweet, buffer_forward, fetch_request,
                              fetch_response, print_forward);
          })
          .ConsumeValueOrDie()
          .Run();
  CHECK_EQ(fluent::Status::OK, status);
}
