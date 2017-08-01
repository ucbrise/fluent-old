#include <cstdint>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/file_util.h"
#include "common/status.h"
#include "examples/file_system/file_system.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/pqxx_client.h"
#include "ra/logical/all.h"
#include "zmq_util/zmq_util.h"

namespace lra = fluent::ra::logical;
namespace ldb = fluent::lineagedb;
namespace zmq_util = fluent::zmq_util;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 7) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <file_system_address> \\" << std::endl   //
              << "  <address> \\" << std::endl               //
              << "  <lineage_file> \\" << std::endl;
    return 1;
  }

  const std::string db_user = argv[1];
  const std::string db_pass = argv[2];
  const std::string db_dbname = argv[3];
  const std::string file_system_address = argv[4];
  const std::string address = argv[5];
  const std::string lineage_file = argv[6];

  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REQ);
  socket.connect(file_system_address);

  ldb::ConnectionConfig config{"localhost", 5432, db_user, db_pass, db_dbname};
  auto fb = fluent::fluent<ldb::PqxxClient>("file_system_server", address,
                                            &context, config);
  auto fe =
      AddFileSystemApi(std::move(fb).ConsumeValueOrDie())
          .RegisterRules([&](auto& write_req, auto& write_resp, auto& read_req,
                             auto& read_resp) {
            using namespace fluent::infix;

            auto write =
                write_resp <=
                (lra::make_collection(&write_req) |
                 lra::map([&](const write_req_tuple& t) -> write_resp_tuple {
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t id = std::get<2>(t);
                   const int start = std::get<3>(t);
                   const std::string& data = std::get<4>(t);

                   std::string msg = fmt::format("write {} {}", start, data);
                   zmq_util::send_string(msg, &socket);
                   zmq_util::recv_string(&socket);
                   return std::make_tuple(src_addr, id);
                 }));

            auto read =
                read_resp <=
                (lra::make_collection(&read_req) |
                 lra::map([&](const read_req_tuple& t) -> read_resp_tuple {
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t id = std::get<2>(t);
                   const int start = std::get<3>(t);
                   const int stop = std::get<4>(t);

                   std::string msg = fmt::format("read {} {}", start, stop);
                   zmq_util::send_string(msg, &socket);
                   std::string data = zmq_util::recv_string(&socket);
                   return std::make_tuple(src_addr, id, std::move(data));
                 }));

            return std::make_tuple(write, read);
          })
          .ConsumeValueOrDie();
  const std::string script =
      fluent::common::Slurp(lineage_file).ConsumeValueOrDie();
  CHECK_EQ(fluent::common::Status::OK,
           fe.RegisterBlackBoxPythonLineageScript(script));
  CHECK_EQ(fluent::common::Status::OK,
           (fe.RegisterBlackBoxPythonLineage<2, 3>("read_lineage")));
  CHECK_EQ(fluent::common::Status::OK, fe.Run());
}
