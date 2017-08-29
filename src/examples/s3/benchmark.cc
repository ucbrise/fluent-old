#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

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
#include "common/rand_util.h"
#include "fmt/format.h"
#include "glog/logging.h"

namespace S3 = Aws::S3;

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

int main() {
  Aws::SDKOptions options;
  Aws::InitAPI(options);
  S3::S3Client client;

  // Create a benchmark bucket.
  {
    S3::Model::CreateBucketRequest request;
    request.SetBucket("mwhittaker_benchmark");
    S3::Model::CreateBucketOutcome outcome = client.CreateBucket(request);
    GetResultOrDie(&outcome);
  }

  using namespace std::chrono;
  time_point<system_clock> start = system_clock::now();

  // Create objects.
  const int num_objects = 25;
  for (int i = 0; i < num_objects; ++i) {
    std::shared_ptr<Aws::IOStream> ss = std::make_shared<std::stringstream>();
    const std::string content = fluent::common::RandomAlphanum(1024);
    (*ss) << content;

    S3::Model::PutObjectRequest request;
    request.SetBucket("mwhittaker_benchmark");
    request.SetKey(fmt::format("{:>04}.txt", i).c_str());
    request.SetBody(ss);
    S3::Model::PutObjectOutcome outcome = client.PutObject(request);
    GetResultOrDie(&outcome);
  }

  // Get objects.
  for (int i = 0; i < num_objects; ++i) {
    S3::Model::GetObjectRequest request;
    request.SetBucket("mwhittaker_benchmark");
    request.SetKey(fmt::format("{:>04}.txt", i).c_str());
    S3::Model::GetObjectOutcome outcome = client.GetObject(request);
    GetResultOrDie(&outcome);
  }

  // Remove objects.
  for (int i = 0; i < num_objects; ++i) {
    S3::Model::DeleteObjectRequest request;
    request.SetBucket("mwhittaker_benchmark");
    request.SetKey(fmt::format("{:>04}.txt", i).c_str());
    S3::Model::DeleteObjectOutcome outcome = client.DeleteObject(request);
    GetResultOrDie(&outcome);
  }

  nanoseconds elapsed = system_clock::now() - start;
  double seconds = elapsed.count() / 1e9;
  double frequency = static_cast<double>(num_objects * 3) / seconds;
  std::cout << fmt::format("{},{},{}", num_objects, seconds, frequency)
            << std::endl;

  // Remove the bucket.
  {
    S3::Model::DeleteBucketRequest request;
    request.SetBucket("mwhittaker_benchmark");
    S3::Model::DeleteBucketOutcome outcome = client.DeleteBucket(request);
    GetResultOrDie(&outcome);
  }

  Aws::ShutdownAPI(options);
}
