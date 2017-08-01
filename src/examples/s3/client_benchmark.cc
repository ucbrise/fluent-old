#include <cstdint>

#include <map>
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
#include "fmt/format.h"
#include "glog/logging.h"
#include "zmq.hpp"

#include "common/macros.h"
#include "common/mock_pickler.h"
#include "common/rand_util.h"
#include "common/status.h"
#include "common/string_util.h"
#include "examples/s3/api_benchmark.h"
#include "fluent/fluent.h"

namespace S3 = Aws::S3;
namespace ldb = fluent::lineagedb;
namespace lra = fluent::ra::logical;

std::string AwsStringToString(const Aws::String& s) {
  return std::string(s.begin(), s.end());
}

template <typename R, typename E>
R GetResultOrDie(Aws::Utils::Outcome<R, Aws::Client::AWSError<E>>* outcome) {
  if (outcome->IsSuccess()) {
    return outcome->GetResultWithOwnership();
  }

  const Aws::Client::AWSError<E>& error = outcome->GetError();
  const std::string exn_name = AwsStringToString(error.GetExceptionName());
  const std::string message = AwsStringToString(error.GetMessage());
  const std::string err =
      fmt::format("ExceptionName = '{}'\nMessage = '{}'", exn_name, message);
  throw std::runtime_error(err);
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 3) {
    std::cerr << "usage: " << argv[0] << " \\" << std::endl  //
              << "  <server_address> \\" << std::endl        //
              << "  <client_address> \\" << std::endl;
    return 1;
  }

  // Command line arguments.
  const std::string server_addr = argv[1];
  const std::string client_addr = argv[2];

  // Random id generator.
  fluent::common::RandomIdGenerator id_gen;

  // ZeroMQ context.
  zmq::context_t context(1);

  std::vector<echo_req_tuple> echo_req_t;
  std::vector<rm_req_tuple> rm_req_t;
  std::vector<cat_req_tuple> cat_req_t;

  ldb::ConnectionConfig confg;
  const std::string name = "s3_client_benchmark";
  auto fb = fluent::fluent<ldb::NoopClient, fluent::common::Hash, ldb::ToSql,
                           fluent::common::MockPickler>(name, client_addr,
                                                        &context, confg)
                .ConsumeValueOrDie();
  auto f =
      AddS3Api(std::move(fb))
          .RegisterRules([&](auto& echo_req, auto& echo_resp, auto& rm_req,
                             auto& rm_resp, auto& cat_req, auto& cat_resp) {
            UNUSED(echo_resp);
            UNUSED(rm_resp);
            UNUSED(cat_resp);

            using namespace fluent::infix;
            auto send_echo = echo_req <= lra::make_iterable(&echo_req_t);
            auto send_rm = rm_req <= lra::make_iterable(&rm_req_t);
            auto send_cat = cat_req <= lra::make_iterable(&cat_req_t);
            return std::make_tuple(send_echo, send_rm, send_cat);
          })
          .ConsumeValueOrDie();

  // Connect to S3.
  Aws::SDKOptions options;
  Aws::InitAPI(options);
  S3::S3Client client;
  const std::string bucket = "mwhittaker_benchmark";

  // Create bucket.
  {
    S3::Model::CreateBucketRequest request;
    request.SetBucket(bucket.c_str());
    S3::Model::CreateBucketOutcome outcome = client.CreateBucket(request);
    GetResultOrDie(&outcome);
  }

  using namespace std::chrono;
  time_point<system_clock> start = system_clock::now();

  // Create objects.
  const int num_objects = 25;
  for (int i = 0; i < num_objects; ++i) {
    echo_req_t.clear();
    const std::int64_t id = id_gen.Generate();
    std::string key = fmt::format("{:>04}.txt", i);
    std::string part = fluent::common::RandomAlphanum(1024);
    echo_req_t.push_back({server_addr, client_addr, id, bucket, key, part});

    CHECK_EQ(f.Tick(), fluent::common::Status::OK);
    CHECK_EQ(f.Receive(), fluent::common::Status::OK);
  }
  echo_req_t.clear();

  // Read objects.
  for (int i = 0; i < num_objects; ++i) {
    cat_req_t.clear();
    const std::int64_t id = id_gen.Generate();
    std::string key = fmt::format("{:>04}.txt", i);
    cat_req_t.push_back({server_addr, client_addr, id, bucket, key});

    CHECK_EQ(f.Tick(), fluent::common::Status::OK);
    CHECK_EQ(f.Receive(), fluent::common::Status::OK);
  }
  cat_req_t.clear();

  // Remove objects.
  for (int i = 0; i < num_objects; ++i) {
    rm_req_t.clear();
    const std::int64_t id = id_gen.Generate();
    std::string key = fmt::format("{:>04}.txt", i);
    rm_req_t.push_back({server_addr, client_addr, id, bucket, key});

    CHECK_EQ(f.Tick(), fluent::common::Status::OK);
    CHECK_EQ(f.Receive(), fluent::common::Status::OK);
  }
  rm_req_t.clear();

  nanoseconds elapsed = system_clock::now() - start;
  double seconds = elapsed.count() / 1e9;
  double frequency = static_cast<double>(num_objects * 3) / seconds;
  std::cout << fmt::format("{},{},{}", num_objects, seconds, frequency)
            << std::endl;
}
