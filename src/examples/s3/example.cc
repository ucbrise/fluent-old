#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

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
  options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;
  Aws::InitAPI(options);

  S3::S3Client client;

  // Create a bucket.
  {
    S3::Model::CreateBucketRequest request;
    request.SetBucket("mwhittakertest");
    S3::Model::CreateBucketOutcome outcome = client.CreateBucket(request);
    GetResultOrDie(&outcome);
  }

  // Create an object.
  {
    std::shared_ptr<Aws::IOStream> ss = std::make_shared<std::stringstream>();
    (*ss) << "hello, world\n";

    S3::Model::PutObjectRequest request;
    request.SetBucket("mwhittakertest");
    request.SetKey("test.txt");
    request.SetBody(ss);
    S3::Model::PutObjectOutcome outcome = client.PutObject(request);
    GetResultOrDie(&outcome);
  }

  // Copy the object.
  {
    S3::Model::CopyObjectRequest request;
    request.SetBucket("mwhittakertest");
    request.SetCopySource("mwhittakertest/test.txt");
    request.SetKey("test2.txt");
    S3::Model::CopyObjectOutcome outcome = client.CopyObject(request);
    GetResultOrDie(&outcome);
  }

  // List objects.
  {
    S3::Model::ListObjectsV2Request request;
    request.SetBucket("mwhittakertest");
    S3::Model::ListObjectsV2Outcome outcome = client.ListObjectsV2(request);
    S3::Model::ListObjectsV2Result result = GetResultOrDie(&outcome);
    for (const S3::Model::Object& o : result.GetContents()) {
      std::cout << o.GetKey() << std::endl;
    }
  }

  // Get the object.
  {
    S3::Model::GetObjectRequest request;
    request.SetBucket("mwhittakertest");
    request.SetKey("test.txt");
    S3::Model::GetObjectOutcome outcome = client.GetObject(request);
    S3::Model::GetObjectResult result = GetResultOrDie(&outcome);
    std::cout << result.GetBody().rdbuf() << std::endl;
  }

  // Remove the objects.
  {
    S3::Model::DeleteObjectRequest request;
    request.SetBucket("mwhittakertest");
    request.SetKey("test.txt");
    S3::Model::DeleteObjectOutcome outcome = client.DeleteObject(request);
    GetResultOrDie(&outcome);

    request.SetBucket("mwhittakertest");
    request.SetKey("test2.txt");
    outcome = client.DeleteObject(request);
    GetResultOrDie(&outcome);
  }

  // Remove the bucket.
  {
    S3::Model::DeleteBucketRequest request;
    request.SetBucket("mwhittakertest");
    S3::Model::DeleteBucketOutcome outcome = client.DeleteBucket(request);
    GetResultOrDie(&outcome);
  }

  Aws::ShutdownAPI(options);
}
