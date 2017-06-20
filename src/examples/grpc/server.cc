#include <memory>
#include <string>

#include "glog/logging.h"
#include "grpc++/grpc++.h"

#include "common/macros.h"
#include "examples/grpc/api.grpc.pb.h"

class EchoServiceImpl : public EchoService::Service {
  grpc::Status Echo(grpc::ServerContext* context, const EchoRequest* request,
                    EchoReply* reply) override {
    UNUSED(context);
    reply->set_msg(request->msg());
    return grpc::Status::OK;
  }
};

int main(int, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  const std::string address = "0.0.0.0:9000";
  EchoServiceImpl service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  LOG(INFO) << "Server listening on " << address;
  server->Wait();
}
