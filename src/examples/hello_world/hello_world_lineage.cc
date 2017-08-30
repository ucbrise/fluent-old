#include <iostream>

#include "glog/logging.h"
#include "zmq.hpp"

#include "common/rand_util.h"
#include "fluent/fluent.h"

using namespace fluent::common;
using namespace fluent::infix;
using namespace fluent::lineagedb;
using namespace fluent::ra::logical;
using namespace fluent;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 4) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl;
    return 1;
  }

  const std::string name = "hello_world_" + RandomAlphanum(10);
  const std::string addr = "tcp://0.0.0.0:8000";
  ConnectionConfig conf;
  conf.host = "localhost";
  conf.port = 5432;
  conf.user = argv[1];
  conf.password = argv[2];
  conf.dbname = argv[3];
  zmq::context_t context(1);

  auto f = fluent<PqxxClient>(name, addr, &context, conf)
               .ConsumeValueOrDie()
               .stdin()
               .stdout()
               .RegisterRules([](auto& in, auto& out) {
                 return std::make_tuple(out <= make_collection(&in));
               })
               .ConsumeValueOrDie();
  CHECK_EQ(f.Run(), Status::OK);
}
