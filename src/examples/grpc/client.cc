#include <iostream>
#include <memory>
#include <string>

#include "glog/logging.h"
#include "grpc++/grpc++.h"

#include "examples/grpc/api.grpc.pb.h"

// using grpc::Channel;
// using grpc::ClientContext;
// using grpc::Status;
// using helloworld::HelloRequest;
// using helloworld::HelloReply;
// using helloworld::Greeter;

class EchoClient {
 public:
  EchoClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(EchoService::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string Echo(const std::string& msg) {
    // Data we are sending to the server.
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
