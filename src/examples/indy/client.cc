#include <iostream>

#include "glog/logging.h"

#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "fluent/serialization.h"
#include "ra/all.h"

namespace ra = fluent::ra;

using address_t = std::string;
using server_address_t = std::string;
using client_address_t = std::string;
using k_t = std::string;
using v_t = int;
using succeed_t = std::string;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 3) {
    std::cerr << "usage: " << argv[0]
              << " <server address> <client address>" << std::endl;
    return 1;
  }

  const std::string server_address = argv[1];
  const std::string client_address = argv[2];
  zmq::context_t context(1);

  auto f =
      fluent::fluent(client_address, &context)
          // Accept GET/PUT from stdin
          .stdin()
          // Show results to stdout
          .stdout()
          // Send GET request
          .channel<server_address_t, client_address_t, k_t>("getrequest")
          // Receive GET response
          .channel<client_address_t, v_t>("getresponse")
          // Send PUT request
          .channel<server_address_t, client_address_t, k_t, v_t>("putrequest")
          // Receive PUT response
          .channel<client_address_t, succeed_t>("putresponse")
          .RegisterRules([&](auto& in, auto& out, auto& getrequest, auto& getresponse, auto& putrequest, auto& putresponse) {
            using namespace fluent::infix;
            auto send_get =
                getrequest <=
                (in.Iterable() | ra::filter([](const std::tuple<std::string>& line) {
                                   std::stringstream ss(std::get<0>(line));
                                   std::istream_iterator<std::string> begin(ss);
                                   std::istream_iterator<std::string> end;
                                   std::vector<std::string> vstrings(begin, end);
                                   return vstrings[0] == "GET";
                                 })
                               | ra::map([&](const std::tuple<std::string>& line) {
                                   std::stringstream ss(std::get<0>(line));
                                   std::istream_iterator<std::string> begin(ss);
                                   std::istream_iterator<std::string> end;
                                   std::vector<std::string> vstrings(begin, end);
                                   return std::make_tuple(server_address, client_address, vstrings[1]);
                                 }));
            auto send_put =
                putrequest <=
                (in.Iterable() | ra::filter([](const std::tuple<std::string>& line) {
                                   std::stringstream ss(std::get<0>(line));
                                   std::istream_iterator<std::string> begin(ss);
                                   std::istream_iterator<std::string> end;
                                   std::vector<std::string> vstrings(begin, end);
                                   return vstrings[0] == "PUT";
                                 })
                               | ra::map([&](const std::tuple<std::string>& line) {
                                   std::stringstream ss(std::get<0>(line));
                                   std::istream_iterator<std::string> begin(ss);
                                   std::istream_iterator<std::string> end;
                                   std::vector<std::string> vstrings(begin, end);
                                   return std::make_tuple(server_address, client_address, vstrings[1], fluent::FromString<int>(vstrings[2]));
                                 }));

            auto receive_get = out <= (getresponse.Iterable() | ra::project<1>()
                                                              | ra::map([](const std::tuple<int>& t) {
                                                                return std::make_tuple(fluent::ToString(std::get<0>(t)));
                                                              }));

            auto receive_put = out <= (putresponse.Iterable() | ra::project<1>());

            return std::make_tuple(send_get, send_put, receive_get, receive_put);
          });

  f.Run();
}