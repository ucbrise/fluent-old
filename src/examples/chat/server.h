#ifndef EXAMPLES_CHAT_SERVER_H_
#define EXAMPLES_CHAT_SERVER_H_

#include "postgres/client.h"

struct ServerArgs {
  std::string server_address;
};

int ServerMain(const ServerArgs& args,
               fluent::postgres::Client* postgres_client);

#endif  // EXAMPLES_CHAT_SERVER_H_
