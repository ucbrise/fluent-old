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
#include "aws/s3/model/CopyObjectRequest.h"
#include "aws/s3/model/CopyObjectResult.h"
#include "aws/s3/model/CreateBucketRequest.h"
#include "aws/s3/model/CreateBucketResult.h"
#include "aws/s3/model/DeleteBucketRequest.h"
#include "aws/s3/model/DeleteObjectRequest.h"
#include "aws/s3/model/DeleteObjectResult.h"
#include "aws/s3/model/GetObjectRequest.h"
#include "aws/s3/model/GetObjectResult.h"
#include "aws/s3/model/ListObjectsV2Request.h"
#include "aws/s3/model/ListObjectsV2Result.h"
#include "aws/s3/model/PutObjectRequest.h"
#include "aws/s3/model/PutObjectResult.h"
#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/file_util.h"
#include "common/macros.h"
#include "common/rand_util.h"
#include "examples/s3/api.h"
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

  if (argc != 6) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <db_user> \\" << std::endl               //
              << "  <db_password> \\" << std::endl           //
              << "  <db_dbname> \\" << std::endl             //
              << "  <address> \\" << std::endl               //
              << "  <lineage_file> \\" << std::endl          //
        ;
    return 1;
  }

  // Command line arguments.
  const std::string db_user = argv[1];
  const std::string db_password = argv[2];
  const std::string db_dbname = argv[3];
  const std::string addr = argv[4];
  const std::string lineage_file = argv[5];

  // ZeroMQ socket.
  zmq::context_t context(1);

  // Initialize S3 connection.
  Aws::SDKOptions options;
  options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;
  Aws::InitAPI(options);
  S3::S3Client client;

  // Lineage database configuration.
  ldb::ConnectionConfig conf;
  conf.host = "localhost";
  conf.port = 5432;
  conf.user = db_user;
  conf.password = db_password;
  conf.dbname = db_dbname;

  // Fluent builder.
  const std::string name = "s3_server";
  auto fb_or = fluent::fluent<ldb::PqxxClient>(name, addr, &context, conf);
  auto fb = fb_or.ConsumeValueOrDie();

  // Declare collections.
  auto with_collections = AddS3Api(std::move(fb));

  // Register rules.
  auto with_rules_or =
      std::move(with_collections)
          .RegisterRules([&](auto& mb_req, auto& mb_resp, auto& rb_req,
                             auto& rb_resp, auto& echo_req, auto& echo_resp,
                             auto& rm_req, auto& rm_resp, auto& ls_req,
                             auto& ls_resp, auto& cat_req, auto& cat_resp,
                             auto& cp_req, auto& cp_resp) {
            using namespace fluent::infix;

            auto mb = mb_resp <=
                      (lra::make_collection(&mb_req) |
                       lra::map([&](const mb_req_tuple& t) -> mb_resp_tuple {
                         // const std::string& dst_addr = std::get<0>(t);
                         const std::string& src_addr = std::get<1>(t);
                         const std::int64_t& id = std::get<2>(t);
                         const std::string& bucket = std::get<3>(t);

                         S3::Model::CreateBucketRequest request;
                         request.SetBucket(bucket.c_str());
                         S3::Model::CreateBucketOutcome outcome =
                             client.CreateBucket(request);
                         const std::string err = GetErrorString(outcome);
                         return {src_addr, id, err};
                       }));

            auto rb = rb_resp <=
                      (lra::make_collection(&rb_req) |
                       lra::map([&](const rb_req_tuple& t) -> rb_resp_tuple {
                         // const std::string& dst_addr = std::get<0>(t);
                         const std::string& src_addr = std::get<1>(t);
                         const std::int64_t& id = std::get<2>(t);
                         const std::string& bucket = std::get<3>(t);

                         S3::Model::DeleteBucketRequest request;
                         request.SetBucket(bucket.c_str());
                         S3::Model::DeleteBucketOutcome outcome =
                             client.DeleteBucket(request);
                         const std::string err = GetErrorString(outcome);
                         return {src_addr, id, err};
                       }));

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

            auto ls = ls_resp <=
                      (lra::make_collection(&ls_req) |
                       lra::map([&](const ls_req_tuple& t) -> ls_resp_tuple {
                         // const std::string& dst_addr = std::get<0>(t);
                         const std::string& src_addr = std::get<1>(t);
                         const std::int64_t& id = std::get<2>(t);
                         const std::string& bucket = std::get<3>(t);

                         S3::Model::ListObjectsV2Request request;
                         request.SetBucket(bucket.c_str());
                         S3::Model::ListObjectsV2Outcome outcome =
                             client.ListObjectsV2(request);

                         std::vector<std::string> keys;
                         std::string err;
                         if (outcome.IsSuccess()) {
                           for (const S3::Model::Object& o :
                                outcome.GetResult().GetContents()) {
                             keys.push_back(o.GetKey().c_str());
                           }
                         } else {
                           err = GetErrorString(outcome);
                         }

                         return {src_addr, id, keys, err};
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

            auto cp =
                cp_resp <=
                (lra::make_collection(&cp_req) |
                 lra::map([&](const cp_req_tuple& t) -> cp_resp_tuple {
                   // const std::string& dst_addr = std::get<0>(t);
                   const std::string& src_addr = std::get<1>(t);
                   const std::int64_t& id = std::get<2>(t);
                   const std::string& src_bucket = std::get<3>(t);
                   const std::string& src_key = std::get<4>(t);
                   const std::string& dst_bucket = std::get<5>(t);
                   const std::string& dst_key = std::get<6>(t);

                   S3::Model::CopyObjectRequest request;
                   request.SetCopySource((src_bucket + "/" + src_key).c_str());
                   request.SetBucket(dst_bucket.c_str());
                   request.SetKey(dst_key.c_str());
                   S3::Model::CopyObjectOutcome outcome =
                       client.CopyObject(request);
                   const std::string err = GetErrorString(outcome);
                   return {src_addr, id, err};
                 }));

            return std::make_tuple(mb, rb, echo, rm, ls, cat, cp);
          });
  auto with_rules = with_rules_or.ConsumeValueOrDie();

  const std::string script = fluent::Slurp(lineage_file).ConsumeValueOrDie();
  CHECK_EQ(fluent::Status::OK,
           with_rules.RegisterBlackBoxPythonLineageScript(script));
  CHECK_EQ(fluent::Status::OK,
           (with_rules.RegisterBlackBoxPythonLineage<2, 3>("rb_lineage")));
  CHECK_EQ(fluent::Status::OK,
           (with_rules.RegisterBlackBoxPythonLineage<4, 5>("echo_lineage")));
  CHECK_EQ(fluent::Status::OK,
           (with_rules.RegisterBlackBoxPythonLineage<6, 7>("rm_lineage")));
  CHECK_EQ(fluent::Status::OK,
           (with_rules.RegisterBlackBoxPythonLineage<8, 9>("ls_lineage")));
  CHECK_EQ(fluent::Status::OK,
           (with_rules.RegisterBlackBoxPythonLineage<10, 11>("cat_lineage")));
  CHECK_EQ(fluent::Status::OK,
           (with_rules.RegisterBlackBoxPythonLineage<12, 13>("cp_lineage")));

  CHECK_EQ(with_rules.Run(), fluent::Status::OK);

  return 0;
}
