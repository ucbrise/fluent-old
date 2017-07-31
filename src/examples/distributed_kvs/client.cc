#include "examples/distributed_kvs/client.h"

bool KeyValueServiceClient::Set(const std::string& key,
                                const google::protobuf::int32 value,
                                google::protobuf::int64 id,
                                google::protobuf::int64 timestamp) {
  // Request.
  SetRequest request;
  request.set_key(key);
  request.set_value(value);
  request.set_id(id);
  request.set_timestamp(timestamp);

  // Reply.
  SetReply reply;

  // RPC.
  grpc::ClientContext context;
  grpc::Status status = stub_->Set(&context, request, &reply);
  CHECK(status.ok()) << status.error_message();
  return reply.success();
}

std::tuple<google::protobuf::int32, google::protobuf::int64>
KeyValueServiceClient::Get(const std::string& key) {
  // Request.
  GetRequest request;
  request.set_key(key);

  // Reply.
  GetReply reply;

  // RPC.
  grpc::ClientContext context;
  grpc::Status status = stub_->Get(&context, request, &reply);
  CHECK(status.ok()) << status.error_message();
  return {reply.value(), reply.id()};
}

bool KeyValueServiceClient::Merge(
    const std::vector<
        std::tuple<std::string, google::protobuf::int32,
                   google::protobuf::int64, google::protobuf::int64>>& delta) {
  // Request.
  MergeRequest request;
  for (const std::tuple<std::string, google::protobuf::int32,
                        google::protobuf::int64, google::protobuf::int64>& t :
       delta) {
    request.add_key(std::get<0>(t));
    request.add_value(std::get<1>(t));
    request.add_id(std::get<2>(t));
    request.add_timestamp(std::get<3>(t));
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
