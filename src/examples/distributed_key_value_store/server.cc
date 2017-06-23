#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "glog/logging.h"
#include "grpc++/grpc++.h"

#include "common/macros.h"
#include "common/rand_util.h"
#include "examples/distributed_key_value_store/api.grpc.pb.h"
#include "examples/distributed_key_value_store/api.pb.h"
#include "examples/distributed_key_value_store/client.h"

using ::google::protobuf::int64;

std::string MapToString(const std::map<std::string, int64>& m) {
  std::string s = "{";
  for (const std::pair<const std::string, int64>& pair : m) {
    const std::string key_string = pair.first;
    const std::string value_string = std::to_string(pair.second);
    s += "(" + key_string + ", " + value_string + "), ";
  }
  s += "}";
  return s;
}

class KeyValueServiceImpl : public KeyValueService::Service {
 public:
  KeyValueServiceImpl(const std::vector<std::string>& replica_addresses) {
    for (const std::string& replica_address : replica_addresses) {
      std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(
          replica_address, grpc::InsecureChannelCredentials());
      clients_.push_back(KeyValueServiceClient(channel));
    }

    gossip_thread_ = std::thread(&KeyValueServiceImpl::Gossip, this);
  }

 private:
  void WaitForConnected() {
    for (KeyValueServiceClient& client : clients_) {
      using namespace std::chrono;
      time_point<system_clock> deadline = system_clock::now() + seconds(120);
      LOG(INFO) << "Waiting for channel to connect.";
      client.Channel()->WaitForConnected(deadline);
      LOG(INFO) << "Channel connected.";
    }
  }

  void Gossip() {
    WaitForConnected();
    LOG(INFO) << "All channels connected.";

    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(1));

      std::map<std::string, int64> delta;
      {
        std::unique_lock<std::mutex> lock(delta_lock_);
        std::swap(delta, delta_);
      }
      LOG(INFO) << "Gossiping delta = " << MapToString(delta);

      for (KeyValueServiceClient& client : clients_) {
        client.Merge(delta);
      }
    }
  }

  grpc::Status Set(grpc::ServerContext*, const SetRequest* request,
                   SetReply* reply) override {
    LOG(INFO) << "Set:\n" << request->DebugString();
    std::unique_lock<std::mutex> lock(delta_lock_);
    const std::string& key = request->key();
    const int64 value = request->value();
    kvs_[key] = std::max(kvs_[key], value);
    delta_[key] = kvs_[key];
    reply->set_success(true);
    return grpc::Status::OK;
  }

  grpc::Status Get(grpc::ServerContext*, const GetRequest* request,
                   GetReply* reply) override {
    LOG(INFO) << "Get:\n" << request->DebugString();
    reply->set_value(kvs_[request->key()]);
    return grpc::Status::OK;
  }

  grpc::Status Merge(grpc::ServerContext*, const MergeRequest* request,
                     MergeReply* reply) override {
    LOG(INFO) << "Merge:\n" << request->DebugString();
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
  std::thread gossip_thread_;
  std::vector<KeyValueServiceClient> clients_;
  std::mutex delta_lock_;
  std::map<std::string, int64> delta_;
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

  KeyValueServiceImpl service(replica_addresses);
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
