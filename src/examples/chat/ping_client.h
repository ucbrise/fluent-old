#ifndef EXAMPLES_CHAT_PING_CLIENT_H_
#define EXAMPLES_CHAT_PING_CLIENT_H_

#include "postgres/client.h"

struct PingClientArgs {
  std::string server_address;
  std::string client_address;
  std::string nickname;
  std::string msg;
};

int PingClientMain(const PingClientArgs& args,
                   fluent::postgres::Client* postgres_client);

#endif  // EXAMPLES_CHAT_PING_CLIENT_H_
