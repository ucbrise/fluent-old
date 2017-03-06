#include <iostream>

#include "glog/logging.h"

#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "ra/all.h"

namespace ra = fluent::ra;

using address_t = std::string;
using server_address_t = std::string;
using client_address_t = std::string;
using nickname_t = std::string;
using message_t = std::string;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 4) {
    std::cerr << "usage: " << argv[0]
              << " <server address> <client address> <nickname>" << std::endl;
    return 1;
  }

  const std::string server_address = argv[1];
  const std::string client_address = argv[2];
  const std::string nickname = argv[3];
  zmq::context_t context(1);

  auto f =
      fluent::fluent(client_address, &context)
          .stdin()
          .stdout()
          .channel<server_address_t, client_address_t, nickname_t>("connect")
          .channel<address_t, message_t>("mcast")
          .table<int>("bootstrap_dummy")
          .RegisterRules([&](auto& in, auto& out, auto& connect, auto& mcast,
                             auto& dummy) {
            using namespace fluent::infix;

            auto bootstrap_a =
                connect <= (dummy.Iterable() | ra::count() |
                            ra::filter([](const std::tuple<std::size_t>& t) {
                              return std::get<0>(t) == 0;
                            }) |
                            ra::map([&](const std::tuple<std::size_t>&) {
                              return std::make_tuple(server_address,
                                                     client_address, nickname);
                            }));

            auto bootstrap_b =
                dummy <= (dummy.Iterable() | ra::count() |
                          ra::map([](const std::tuple<int>& t) {
                            return std::make_tuple(std::get<0>(t) + 1);
                          }));

            auto from_in =
                mcast <=
                (in.Iterable() |
                 ra::map([&](const std::tuple<std::string>& line) {
                   return std::make_tuple(server_address, std::get<0>(line));
                 }));

            auto to_out = out <= (mcast.Iterable() | ra::project<1>());

            return std::make_tuple(bootstrap_a, bootstrap_b, from_in, to_out);
          });

  f.Run();
}
