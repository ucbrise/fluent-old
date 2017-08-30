#include "glog/logging.h"
#include "zmq.hpp"

#include "fluent/fluent.h"

using namespace fluent::common;
using namespace fluent::infix;
using namespace fluent::lineagedb;
using namespace fluent::ra::logical;
using namespace fluent;

int main(int, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  const std::string name = "hello_world";
  const std::string addr = "tcp://0.0.0.0:8000";
  ConnectionConfig conf;
  zmq::context_t context(1);

  auto f = fluent<NoopClient>(name, addr, &context, conf)
               .ConsumeValueOrDie()
               .stdin()
               .stdout()
               .RegisterRules([](auto& in, auto& out) {
                 return std::make_tuple(out <= make_collection(&in));
               })
               .ConsumeValueOrDie();
  CHECK_EQ(f.Run(), Status::OK);
}
