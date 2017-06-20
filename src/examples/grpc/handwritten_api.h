#ifndef EXAMPLES_GRPC_HANDWRITTEN_API_H_
#define EXAMPLES_GRPC_HANDWRITTEN_API_H_

#include <cstdint>

#include <string>
#include <tuple>
#include <utility>

#include "fluent/infix.h"
#include "grpc++/grpc++.h"

#include "examples/grpc/api.grpc.pb.h"
#include "examples/grpc/api.pb.h"
#include "ra/logical/all.h"

class EchoServiceClient {
 public:
  EchoServiceClient(std::shared_ptr<grpc::Channel> channel)
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

template <typename FluentBuilder>
auto AddEchoServiceApi(FluentBuilder f) {
  return std::move(f)
      .template channel<std::string, std::string, std::int64_t, std::string>(
          "echo_request", {{"dst_addr", "src_addr", "id", "msg"}})
      .template channel<std::string, std::int64_t, std::string>(
          "echo_reply", {{"addr", "id", "msg"}});
}

template <typename FluentBuilder>
auto GetEchoServiceShim(FluentBuilder f, EchoServiceClient* client) {
  return AddEchoServiceApi(std::move(f))
      .RegisterRules([&](auto& echo_request, auto& echo_reply) {
        using namespace fluent::infix;

        auto echo = echo_reply <=
                    (fluent::ra::logical::make_collection(&echo_request) |
                     fluent::ra::logical::map([client](const auto& t) {
                       const std::string& src_addr = std::get<1>(t);
                       const std::int64_t id = std::get<2>(t);
                       const std::string& msg = std::get<3>(t);
                       return std::make_tuple(src_addr, id, client->Echo(msg));
                     }));

        return std::make_tuple(echo);
      });
}

#endif  // EXAMPLES_GRPC_HANDWRITTEN_API_H_
