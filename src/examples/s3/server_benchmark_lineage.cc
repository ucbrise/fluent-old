#include <chrono>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "aws/core/Aws.h"
#include "aws/core/NoResult.h"
#include "aws/core/client/AWSError.h"
#include "aws/core/utils/Outcome.h"
#include "aws/s3/S3Client.h"
#include "aws/s3/model/CreateBucketRequest.h"
#include "aws/s3/model/CreateBucketResult.h"
#include "aws/s3/model/DeleteBucketRequest.h"
#include "aws/s3/model/DeleteObjectRequest.h"
#include "aws/s3/model/DeleteObjectResult.h"
#include "aws/s3/model/GetObjectRequest.h"
#include "aws/s3/model/GetObjectResult.h"
#include "aws/s3/model/PutObjectRequest.h"
#include "aws/s3/model/PutObjectResult.h"
#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/file_util.h"
#include "common/macros.h"
#include "common/rand_util.h"
#include "examples/s3/api_benchmark.h"
#include "fluent/fluent.h"

namespace S3 = Aws::S3;
namespace ldb = fluent::lineagedb;
namespace lra = fluent::ra::logical;

std::string AwsStringToString(const Aws::String& s) {
  return std::string(s.begin(), s.end());
}

template <typename R, typename E>
std::string GetErrorString(
    const Aws::Utils::Outcome<R, Aws::Client::AWSError<E>>& outcome) {
  if (outcome.IsSuccess()) {
    return "";
  }

  const Aws::Client::AWSError<E>& error = outcome.GetError();
  const std::string exn_name = AwsStringToString(error.GetExceptionName());
  const std::string message = AwsStringToString(error.GetMessage());
  const std::string err =
      fmt::format("ExceptionName = {}\nMessage = {}", exn_name, message);
  return err;
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 7) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_host> \\" << std::endl               //
              << "  <db_port> \\" << std::endl               //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <address>" << std::endl                  //
        ;
    return 1;
  }

  // Command line arguments.
  const std::string db_host = argv[1];
  const int db_port = std::stoi(argv[2]);
  const std::string db_user = argv[3];
  const std::string db_password = argv[4];
  const std::string db_dbname = argv[5];
  const std::string addr = argv[6];

  // ZeroMQ socket.
  zmq::context_t context(1);

  // Initialize S3 connection.
  Aws::SDKOptions options;
  options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;
  Aws::InitAPI(options);
  S3::S3Client client;

  // Lineage database connection.
  ldb::ConnectionConfig conf;
  conf.host = db_host;
  conf.port = db_port;
  conf.user = db_user;
  conf.password = db_password;
  conf.dbname = db_dbname;

  // Fluent builder.
  const std::string name = "s3_server_benchmark_lineage";
  auto fb_or = fluent::fluent<ldb::PqxxClient>(name, addr, &context, conf);
  auto fb = fb_or.ConsumeValueOrDie();

  // Declare collections.
  auto with_collections = AddS3Api(std::move(fb));

  // Register rules.
  auto with_rules_or =
      std::move(with_collections)
          .RegisterRules([&](auto& echo_req, auto& echo_resp, auto& rm_req,
                             auto& rm_resp, auto& cat_req, auto& cat_resp) {
            using namespace fluent::infix;

            auto echo =
                echo_resp <=
                (lra::make_collection(&echo_req) |
                 lra::map([&](const echo_req_tuple& t) -> echo_resp_tuple {
                   // const std::string& dst_addr = std::get<0>(t);
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t& id = std::get<2>(t);
                   const std::string& bucket = std::get<3>(t);
                   const std::string& key = std::get<4>(t);
                   const std::string& part = std::get<5>(t);

                   std::shared_ptr<Aws::IOStream> ss =
                       std::make_shared<std::stringstream>();
                   (*ss) << part;
                   S3::Model::PutObjectRequest request;
                   request.SetBucket(bucket.c_str());
                   request.SetKey(key.c_str());
                   request.SetBody(ss);
                   S3::Model::PutObjectOutcome outcome =
                       client.PutObject(request);
                   const std::string err = GetErrorString(outcome);
                   return {src_addr, id, err};
                 }));

            auto rm = rm_resp <=
                      (lra::make_collection(&rm_req) |
                       lra::map([&](const rm_req_tuple& t) -> rm_resp_tuple {
                         // const std::string& dst_addr = std::get<0>(t);
                         const std::string& src_addr = std::get<1>(t);
                         const std::int64_t& id = std::get<2>(t);
                         const std::string& bucket = std::get<3>(t);
                         const std::string& key = std::get<4>(t);

                         S3::Model::DeleteObjectRequest request;
                         request.SetBucket(bucket.c_str());
                         request.SetKey(key.c_str());
                         S3::Model::DeleteObjectOutcome outcome =
                             client.DeleteObject(request);
                         const std::string err = GetErrorString(outcome);
                         return {src_addr, id, err};
                       }));

            auto cat = cat_resp <=
                       (lra::make_collection(&cat_req) |
                        lra::map([&](const cat_req_tuple& t) -> cat_resp_tuple {
                          // const std::string& dst_addr = std::get<0>(t);
                          const std::string& src_addr = std::get<1>(t);
                          const std::int64_t& id = std::get<2>(t);
                          const std::string& bucket = std::get<3>(t);
                          const std::string& key = std::get<4>(t);

                          S3::Model::GetObjectRequest request;
                          request.SetBucket(bucket.c_str());
                          request.SetKey(key.c_str());
                          S3::Model::GetObjectOutcome outcome =
                              client.GetObject(request);

                          std::string success;
                          std::string err;
                          if (outcome.IsSuccess()) {
                            std::stringstream ss;
                            ss << outcome.GetResult().GetBody().rdbuf();
                            success = ss.str();
                          } else {
                            err = GetErrorString(outcome);
                          }

                          return {src_addr, id, success, err};
                        }));

            return std::make_tuple(echo, rm, cat);
          });
  auto with_rules = with_rules_or.ConsumeValueOrDie();
  CHECK_EQ(with_rules.Run(), fluent::Status::OK);
}
