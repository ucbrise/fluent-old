#include <map>
#include <memory>
#include <string>
#include <vector>

#include "glog/logging.h"
#include "grpc++/grpc++.h"

#include "common/macros.h"
#include "examples/distributed_key_value_store/api.grpc.pb.h"
#include "examples/distributed_key_value_store/api.pb.h"

using ::google::protobuf::int64;

class KeyValueServiceImpl : public KeyValueService::Service {
 public:
 private:
  grpc::Status Set(grpc::ServerContext* context, const SetRequest* request,
                   SetReply* reply) override {
    UNUSED(context);
    kvs_[request->key()] = request->value();
    reply->set_success(true);
    return grpc::Status::OK;
  }

  grpc::Status Get(grpc::ServerContext* context, const GetRequest* request,
                   GetReply* reply) override {
    UNUSED(context);
    reply->set_value(kvs_[request->key()]);
    return grpc::Status::OK;
  }

  grpc::Status Merge(grpc::ServerContext* context, const MergeRequest* request,
                     MergeReply* reply) override {
    UNUSED(context);
    CHECK_EQ(request->key_size(), request->value_size());
    for (int i = 0; i < request->key_size(); ++i) {
      const std::string& key = request->key(i);
      const int64 value = request->value(i);
      kvs_[key] = std::max(kvs_[key], value);
    }
    reply->set_success(true);
    return grpc::Status::OK;
  }

  std::map<std::string, int64> kvs_;
};

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <address> [replica_address]..."
              << std::endl;
    return 1;
  }

  const std::string address = argv[1];
  std::vector<std::string> replica_addresses;
  for (int i = 2; i < argc; ++i) {
    replica_addresses.push_back(argv[i]);
  }

  KeyValueServiceImpl service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

  LOG(INFO) << "Server listening on " << address;
  LOG(INFO) << "Replicas listening on the following addresses:";
  for (const std::string& replica_address : replica_addresses) {
    LOG(INFO) << "  - " << replica_address;
  }

  server->Wait();
}
