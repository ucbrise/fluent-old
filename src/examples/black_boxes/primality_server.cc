#include <cstdint>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/status.h"
#include "examples/black_boxes/primality.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/pqxx_client.h"
#include "ra/all.h"

namespace ra = fluent::ra;
namespace ldb = fluent::lineagedb;

namespace {

bool is_prime(int x) {
  if (x < 2) {
    return false;
  }

  for (int i = 2; i < x; ++i) {
    if (x % i == 0) {
      return false;
    }
  }
  return true;
}

}  // namespace

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

  auto f = [](const auto& t) -> std::tuple<std::string, std::int64_t, bool> {
    return {std::get<1>(t), std::get<2>(t), is_prime(std::get<3>(t))};
  };

  zmq::context_t context(1);
  ldb::ConnectionConfig config{"localhost", 5432, argv[1], argv[2], argv[3]};
  auto fe = AddPrimalityApi(fluent::fluent<ldb::PqxxClient>(
                                "primality_server", argv[4], &context, config)
                                .ConsumeValueOrDie())
                .RegisterRules([&](auto& req, auto& resp) {
                  using namespace fluent::infix;
                  auto rule = resp <= (req.Iterable() | ra::map(f));
                  return std::make_tuple(rule);
                })
                .ConsumeValueOrDie();
  fluent::Status status = fe.RegisterBlackBoxLineage<0, 1>(
      [](const std::string& time_inserted, const std::string& x,
         const std::string& is_prime) {
        (void)time_inserted;
        (void)x;
        (void)is_prime;
        return R"(
          SELECT *
          FROM (VALUES (CAST(NULL AS text),
                        CAST(NULL as bigint),
                        CAST(NULL as integer))) AS T
          WHERE false;
        )";
      });
  CHECK_EQ(fluent::Status::OK, status);
  CHECK_EQ(fluent::Status::OK, fe.Run());
}
