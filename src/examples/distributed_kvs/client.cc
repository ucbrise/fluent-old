#include "examples/distributed_kvs/client.h"

bool KeyValueServiceClient::Set(const std::string& key,
                                const google::protobuf::int64 value) {
  // Request.
  SetRequest request;
  request.set_key(key);
  request.set_value(value);

  // Reply.
  SetReply reply;

  // RPC.
  grpc::ClientContext context;
  grpc::Status status = stub_->Set(&context, request, &reply);
  CHECK(status.ok()) << status.error_message();
  return reply.success();
}

google::protobuf::int64 KeyValueServiceClient::Get(const std::string& key) {
  // Request.
  GetRequest request;
  request.set_key(key);

  // Reply.
  GetReply reply;

  // RPC.
  grpc::ClientContext context;
  grpc::Status status = stub_->Get(&context, request, &reply);
  CHECK(status.ok()) << status.error_message();
  return reply.value();
}

bool KeyValueServiceClient::Merge(
    const std::map<std::string, google::protobuf::int64>& kvs) {
  // Request.
  MergeRequest request;
  for (const std::pair<const std::string, google::protobuf::int64>& kv : kvs) {
    request.add_key(kv.first);
    request.add_value(kv.second);
  }
  CHECK_EQ(request.key_size(), request.value_size());

  // Reply.
  MergeReply reply;

  // RPC.
  grpc::ClientContext context;
  grpc::Status status = stub_->Merge(&context, request, &reply);
  CHECK(status.ok()) << status.error_message();
  return reply.success();
}
