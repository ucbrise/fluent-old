#include <iostream>
#include <memory>
#include <string>

#include "glog/logging.h"
#include "grpc++/grpc++.h"

#include "examples/grpc/api.grpc.pb.h"

class EchoClient {
 public:
  EchoClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(EchoService::NewStub(channel)) {}

  std::string Echo(const std::string& msg) {
    EchoRequest request;
    request.set_msg(msg);
    EchoReply reply;
    grpc::ClientContext context;
    grpc::Status status = stub_->Echo(&context, request, &reply);
    CHECK(status.ok());
    return reply.msg();
  }

 private:
  std::unique_ptr<EchoService::Stub> stub_;
};

int main(int, char** argv) {
  google::InitGoogleLogging(argv[0]);
  EchoClient client(
      grpc::CreateChannel("0.0.0.0:9000", grpc::InsecureChannelCredentials()));
  std::string line;
  std::cout << "> " << std::flush;
  while (std::getline(std::cin, line)) {
    std::cout << client.Echo(line) << std::endl;
    std::cout << "> " << std::flush;
  }
}
