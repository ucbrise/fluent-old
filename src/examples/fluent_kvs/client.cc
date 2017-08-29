#include <cstdint>

#include <map>
#include <vector>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/rand_util.h"
#include "common/string_util.h"
#include "fluent/fluent.h"

using namespace std;

namespace lra = fluent::ra::logical;
namespace ldb = fluent::lineagedb;

auto fluent_program(const std::string& name, const std::string& address,
                    zmq::context_t* context,
                    const ldb::ConnectionConfig& config) {
  return fluent::fluent<ldb::PqxxClient>(name, address, context, config)
      .ConsumeValueOrDie();
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 6) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <server_address> \\" << std::endl        //
              << "  <client_address> \\" << std::endl;
    return 1;
  }

  const std::string db_user = argv[1];
  const std::string db_password = argv[2];
  const std::string db_dbname = argv[3];
  const std::string server_addr = argv[4];
  const std::string client_addr = argv[5];

  fluent::common::RandomIdGenerator id_gen;

  zmq::context_t context(1);

  ldb::ConnectionConfig config;
  config.host = "localhost";
  config.port = 5432;
  config.user = db_user;
  config.password = db_password;
  config.dbname = db_dbname;

  auto f =
      fluent_program("key_value_client", client_addr, &context, config)
          .stdin()
          .stdout()
          .scratch<vector<string>>("split", {{"parts"}})
          .channel<string, string, int64_t, string, string>(
              "set_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
          .channel<string, int64_t>(  //
              "set_response", {{"addr", "id"}})
          .channel<string, string, int64_t, string>(  //
              "get_request", {{"dst_addr", "src_addr", "id", "key"}})
          .channel<string, int64_t, string>(  //
              "get_response", {{"addr", "id", "value"}})
          .RegisterRules([&](auto& stdin, auto& stdout, auto& split,
                             auto& set_request, auto& set_response,
                             auto& get_request, auto& get_response) {
            using namespace fluent::infix;

            auto buffer_stdin =
                split <=
                (lra::make_collection(&stdin) | lra::map([](const auto& s) {
                   const string& str = get<0>(s);
                   return make_tuple(fluent::common::Split(str));
                 }));

            auto send_get =
                get_request <=
                (lra::make_collection(&split) |
                 lra::filter([](const auto& parts_tuple) {
                   const vector<string>& parts = get<0>(parts_tuple);
                   return parts.size() == 2 && parts[0] == "GET";
                 }) |
                 lra::map([&](const auto& parts_tuple) {
                   const vector<string>& parts = get<0>(parts_tuple);
                   const string& key = parts[1];
                   int64_t id = id_gen.Generate();
                   return make_tuple(server_addr, client_addr, id, key);
                 }));

            auto send_set =
                set_request <=
                (lra::make_collection(&split) |
                 lra::filter([](const auto& parts_tuple) {
                   const vector<string>& parts = get<0>(parts_tuple);
                   return parts.size() == 3 && parts[0] == "SET";
                 }) |
                 lra::map([&](const auto& parts_tuple) {
                   const vector<string>& parts = get<0>(parts_tuple);
                   const string& key = parts[1];
                   const string& value = parts[2];
                   int64_t id = id_gen.Generate();
                   return make_tuple(server_addr, client_addr, id, key, value);
                 }));

            auto recv_get = stdout <= (lra::make_collection(&get_response) |
                                       lra::project<2>());

            auto recv_set =
                stdout <=
                (lra::make_collection(&set_response) |
                 lra::map([](const auto&) { return tuple<string>("OK"); }));

            return make_tuple(buffer_stdin, send_get, send_set, recv_get,
                              recv_set);
          });
  CHECK_EQ(fluent::common::Status::OK, f.ConsumeValueOrDie().Run());
}
