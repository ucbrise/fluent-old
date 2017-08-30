#include "glog/logging.h"
#include "zmq.hpp"

#include "fluent/fluent.h"

using namespace fluent::common;
using namespace fluent::infix;
using namespace fluent::lineagedb;
using namespace fluent::ra::logical;
using namespace fluent;
using std::tuple;
using std::string;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 4) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl;
    return 1;
  }

  const std::string name = "hello_world";
  const std::string addr = "tcp://0.0.0.0:8000";
  ConnectionConfig conf;
  conf.host = "localhost";
  conf.port = 5432;
  conf.user = argv[1];
  conf.password = argv[2];
  conf.dbname = argv[3];
  zmq::context_t context(1);

  auto f =
      fluent<PqxxClient>(name, addr, &context, conf)
          .ConsumeValueOrDie()
          .stdin()
          .stdout()
          .table<std::string, std::size_t, int>("metadata", {{"c", "h", "t"}})
          .RegisterRules([](auto& in, auto& out, auto& metadata) {
            auto r1 = metadata <=
                      (make_meta_collection(&in) | map([](const auto& t) {
                         const LocalTupleId tid = std::get<1>(t);
                         return std::make_tuple(tid.collection_name, tid.hash,
                                                tid.logical_time_inserted);
                       }));

            auto r2 =
                out <= (make_meta_collection(&in) |
                        map([](const auto& t) { return std::get<0>(t); }));

            return std::make_tuple(r1, r2);
          })
          .ConsumeValueOrDie();
  CHECK_EQ(f.Run(), Status::OK);
}
