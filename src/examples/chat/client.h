#ifndef EXAMPLES_CHAT_CLIENT_H_
#define EXAMPLES_CHAT_CLIENT_H_

#include "postgres/client.h"

struct ClientArgs {
  std::string server_address;
  std::string client_address;
  std::string nickname;
};

int ClientMain(const ClientArgs& args,
               fluent::postgres::Client* postgres_client);

#endif  // EXAMPLES_CHAT_CLIENT_H_
