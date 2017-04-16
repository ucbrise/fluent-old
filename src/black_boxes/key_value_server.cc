#include <cstdint>

#include <map>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

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

  if (argc != 5) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <address> \\" << std::endl;
    return 1;
  }

  std::map<std::string, std::string> kvs;

  zmq::context_t context(1);
  ldb::ConnectionConfig config{"localhost", 5432, argv[1], argv[2], argv[3]};
  fluent::fluent<ldb::PqxxClient>("key_value_server", argv[4], &context, config)
      .channel<std::string, std::string, std::int64_t, std::string,
               std::string>("set_request",
                            {{"dst_addr", "src_addr", "id", "key", "value"}})
      .channel<std::string, std::int64_t>("set_response", {{"addr", "id"}})
      .channel<std::string, std::string, std::int64_t, std::string>(
          "get_request", {{"dst_addr", "src_addr", "id", "key"}})
      .channel<std::string, std::int64_t, std::string>(
          "get_response", {{"addr", "id", "value"}})
      .RegisterRules([&](auto& set_req, auto& set_resp, auto& get_req,
                         auto& get_resp) {
        using namespace fluent::infix;
        auto set =
            set_resp <= (set_req.Iterable() |
                         ra::map([&kvs](const auto& t)
                                     -> std::tuple<std::string, std::int64_t> {
                           kvs[std::get<3>(t)] = std::get<4>(t);
                           return {std::get<1>(t), std::get<2>(t)};
                         }));
        auto get =
            get_resp <=
            (get_req.Iterable() |
             ra::map([&kvs](const auto& t)
                         -> std::tuple<std::string, std::int64_t, std::string> {
               return {std::get<1>(t), std::get<2>(t), kvs[std::get<3>(t)]};
             }));
        return std::make_tuple(set, get);
      })
      .RegisterBlackBoxLineage<0, 1>([](const std::string& time_inserted,
                                        const std::string& key,
                                        const std::string& value) {
        (void)value;
        return fmt::format(R"(
          SELECT
            CAST('key_value_server_set_request' as TEXT), hash, time_inserted
          FROM key_value_server_set_request
          WHERE key = {} AND time_inserted <= {}
          ORDER BY {}
          LIMIT 1;
        )",
                           key, time_inserted, time_inserted);
      })
      .Run();
}
