#include <cstdint>

#include <map>
#include <vector>

#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/rand_util.h"
#include "common/string_util.h"
#include "examples/s3/api.h"
#include "fluent/fluent.h"

namespace lra = fluent::ra::logical;
namespace ldb = fluent::lineagedb;

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

  // Command line arguments.
  const std::string db_user = argv[1];
  const std::string db_password = argv[2];
  const std::string db_dbname = argv[3];
  const std::string server_addr = argv[4];
  const std::string client_addr = argv[5];

  // Random id generator.
  fluent::RandomIdGenerator id_gen;

  // ZeroMQ context.
  zmq::context_t context(1);

  // Lineage database configuration.
  ldb::ConnectionConfig config;
  config.host = "localhost";
  config.port = 5432;
  config.user = db_user;
  config.password = db_password;
  config.dbname = db_dbname;

  const std::string name = "s3_client_" + fluent::RandomAlphanum(10);
  auto fb = fluent::fluent<ldb::PqxxClient>(name, client_addr, &context, config)
                .ConsumeValueOrDie()
                .stdin()
                .stdout()
                .scratch<std::vector<std::string>>("split", {{"parts"}});
  fluent::Status status =
      AddS3Api(std::move(fb))
          .RegisterRules([&](auto& stdin, auto& stdout, auto& split,
                             auto& mb_req, auto& mb_resp, auto& rb_req,
                             auto& rb_resp, auto& echo_req, auto& echo_resp,
                             auto& rm_req, auto& rm_resp, auto& ls_req,
                             auto& ls_resp, auto& cat_req, auto& cat_resp,
                             auto& cp_req, auto& cp_resp) {
            using namespace fluent::infix;

            using parts_tuple = std::tuple<std::vector<std::string>>;

            auto buffer_stdin =
                split <=
                (lra::make_collection(&stdin) |
                 lra::map([](const std::tuple<std::string>& s) -> parts_tuple {
                   const std::string& line = std::get<0>(s);
                   return {fluent::Split(line)};
                 }));

            auto send_mb =
                mb_req <=
                (lra::make_collection(&split) |
                 lra::filter([](const parts_tuple& t) {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   return parts.size() == 2 && parts[0] == "mb";
                 }) |
                 lra::map([&](const parts_tuple& t) -> mb_req_tuple {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   const std::string& bucket = parts[1];
                   const std::int64_t id = id_gen.Generate();
                   return {server_addr, client_addr, id, bucket};
                 }));

            auto print_mb =
                stdout <= (lra::make_collection(&mb_resp) | lra::project<2>());

            auto send_rb =
                rb_req <=
                (lra::make_collection(&split) |
                 lra::filter([](const parts_tuple& t) {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   return parts.size() == 2 && parts[0] == "rb";
                 }) |
                 lra::map([&](const parts_tuple& t) -> rb_req_tuple {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   const std::string& bucket = parts[1];
                   const std::int64_t id = id_gen.Generate();
                   return {server_addr, client_addr, id, bucket};
                 }));

            auto print_rb =
                stdout <= (lra::make_collection(&rb_resp) | lra::project<2>());

            auto send_echo =
                echo_req <=
                (lra::make_collection(&split) |
                 lra::filter([](const parts_tuple& t) {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   return parts.size() == 4 && parts[0] == "echo";
                 }) |
                 lra::map([&](const parts_tuple& t) -> echo_req_tuple {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   const std::string& bucket = parts[1];
                   const std::string& key = parts[2];
                   const std::string& part = parts[3];
                   const std::int64_t id = id_gen.Generate();
                   return {server_addr, client_addr, id, bucket, key, part};
                 }));

            auto print_echo = stdout <= (lra::make_collection(&echo_resp) |
                                         lra::project<2>());

            auto send_rm =
                rm_req <=
                (lra::make_collection(&split) |
                 lra::filter([](const parts_tuple& t) {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   return parts.size() == 3 && parts[0] == "rm";
                 }) |
                 lra::map([&](const parts_tuple& t) -> rm_req_tuple {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   const std::string& bucket = parts[1];
                   const std::string& key = parts[2];
                   const std::int64_t id = id_gen.Generate();
                   return {server_addr, client_addr, id, bucket, key};
                 }));

            auto print_rm =
                stdout <= (lra::make_collection(&rm_resp) | lra::project<2>());

            auto send_ls =
                ls_req <=
                (lra::make_collection(&split) |
                 lra::filter([](const parts_tuple& t) {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   return parts.size() == 2 && parts[0] == "ls";
                 }) |
                 lra::map([&](const parts_tuple& t) -> ls_req_tuple {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   const std::string& bucket = parts[1];
                   const std::int64_t id = id_gen.Generate();
                   return {server_addr, client_addr, id, bucket};
                 }));

            auto print_ls =
                stdout <=
                (lra::make_collection(&ls_resp) |
                 lra::map([](const ls_resp_tuple& t) {
                   // const std::string& addr = std::get<0>(t);
                   // const std::int64_t& id = std::get<1>(t);
                   const std::vector<std::string>& keys = std::get<2>(t);
                   const std::string& err = std::get<3>(t);

                   if (err != "") {
                     return std::tuple<std::string>(err);
                   } else {
                     return std::tuple<std::string>(fluent::Join(keys));
                   }
                 }));

            auto send_cat =
                cat_req <=
                (lra::make_collection(&split) |
                 lra::filter([](const parts_tuple& t) {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   return parts.size() == 3 && parts[0] == "cat";
                 }) |
                 lra::map([&](const parts_tuple& t) -> cat_req_tuple {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   const std::string& bucket = parts[1];
                   const std::string& key = parts[2];
                   const std::int64_t id = id_gen.Generate();
                   return {server_addr, client_addr, id, bucket, key};
                 }));

            auto print_cat =
                stdout <= (lra::make_collection(&cat_resp) |
                           lra::map([](const cat_resp_tuple& t) {
                             // const std::string& addr = std::get<0>(t);
                             // const std::int64_t& id = std::get<1>(t);
                             const std::string& part = std::get<2>(t);
                             const std::string& err = std::get<3>(t);

                             if (err != "") {
                               return std::tuple<std::string>(err);
                             } else {
                               return std::tuple<std::string>(part);
                             }
                           }));

            auto send_cp =
                cp_req <=
                (lra::make_collection(&split) |
                 lra::filter([](const parts_tuple& t) {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   return parts.size() == 5 && parts[0] == "cp";
                 }) |
                 lra::map([&](const parts_tuple& t) -> cp_req_tuple {
                   const std::vector<std::string>& parts = std::get<0>(t);
                   const std::string& src_bucket = parts[1];
                   const std::string& src_key = parts[2];
                   const std::string& dst_bucket = parts[3];
                   const std::string& dst_key = parts[4];
                   const std::int64_t id = id_gen.Generate();
                   return {server_addr, client_addr, id,     src_bucket,
                           src_key,     dst_bucket,  dst_key};
                 }));

            auto print_cp =
                stdout <= (lra::make_collection(&cp_resp) | lra::project<2>());

            return std::make_tuple(buffer_stdin, send_mb, print_mb, send_rb,
                                   print_rb, send_echo, print_echo, send_rm,
                                   print_rm, send_ls, print_ls, send_cat,
                                   print_cat, send_cp, print_cp);
          })
          .ConsumeValueOrDie()
          .Run();
  CHECK_EQ(status, fluent::Status::OK);
}
