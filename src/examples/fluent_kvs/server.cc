#include <cstdint>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "fluent/fluent.h"

using namespace std;
using namespace fluent::infix;
using namespace fluent::ra;
using namespace fluent::ra::logical;
using namespace fluent::lineagedb;

auto fluent_program(const std::string& name, const std::string& address,
                    zmq::context_t* context, const ConnectionConfig& config) {
  return fluent::fluent<PqxxClient>(name, address, context, config)
      .ConsumeValueOrDie();
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 5) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <address> \\" << std::endl;
    return 1;
  }

  const std::string db_user = argv[1];
  const std::string db_password = argv[2];
  const std::string db_dbname = argv[3];
  const std::string address = argv[4];

  zmq::context_t context(1);

  ConnectionConfig config;
  config.host = "localhost";
  config.port = 5432;
  config.user = db_user;
  config.password = db_password;
  config.dbname = db_dbname;

  auto f =
      fluent_program("key_value_server", address, &context, config)
          .table<string, string>(  //
              "kvs", {{"key", "value"}})
          .channel<string, string, int64_t, string, string>(
              "set_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
          .channel<string, int64_t>(  //
              "set_response", {{"addr", "id"}})
          .channel<string, string, int64_t, string>(  //
              "get_request", {{"dst_addr", "src_addr", "id", "key"}})
          .channel<string, int64_t, string>(  //
              "get_response", {{"addr", "id", "value"}})
          .RegisterRules([](auto& kvs, auto& set_request, auto& set_response,
                            auto& get_request, auto& get_response) {
            auto get =
                get_response <=
                (make_hash_join<LeftKeys<0>, RightKeys<3>>(
                     make_collection(&kvs), make_collection(&get_request)) |
                 project<3, 4, 1>());

            auto set_delete = kvs -=
                (make_hash_join<LeftKeys<0>, RightKeys<3>>(
                     make_collection(&kvs), make_collection(&set_request)) |
                 project<0, 1>());

            auto set_add = kvs +=
                (make_collection(&set_request) | project<3, 4>());

            auto set_ack = set_response <=
                           (make_collection(&set_request) | project<1, 2>());

            return make_tuple(get, set_delete, set_add, set_ack);
          });
  CHECK_EQ(fluent::Status::OK, f.ConsumeValueOrDie().Run());
}
