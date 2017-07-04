#ifndef EXAMPLES_DISTRIBUTED_KVS_CLIENT_H_
#define EXAMPLES_DISTRIBUTED_KVS_CLIENT_H_

#include <iostream>
#include <memory>
#include <string>
#include <tuple>

#include "glog/logging.h"
#include "grpc++/grpc++.h"

#include "examples/distributed_kvs/api.grpc.pb.h"

class KeyValueServiceClient {
 public:
  KeyValueServiceClient(std::shared_ptr<grpc::Channel> channel)
      : channel_(channel), stub_(KeyValueService::NewStub(channel)) {}

  bool Set(const std::string& key, const google::protobuf::int32 value,
           google::protobuf::int64 id, google::protobuf::int64 timestamp);

  std::tuple<google::protobuf::int32, google::protobuf::int64> Get(
      const std::string& key);

  bool Merge(const std::vector<std::tuple<std::string, google::protobuf::int32,
                                          google::protobuf::int64,
                                          google::protobuf::int64>>& delta);

  std::shared_ptr<grpc::Channel> Channel() { return channel_; }

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<KeyValueService::Stub> stub_;
};

#endif  // EXAMPLES_DISTRIBUTED_KVS_CLIENT_H_
