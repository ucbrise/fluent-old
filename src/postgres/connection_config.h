#ifndef POSTGRES_CONNECTION_CONFIG_H_
#define POSTGRES_CONNECTION_CONFIG_H_

#include <cstdint>

#include <string>

namespace fluent {
namespace postgres {

// PostgreSQL connections parameters. Each field corresponds to a connection
// parameter documented here:
// https://www.postgresql.org/docs/9.3/static/libpq-connect.html#LIBPQ-PARAMKEYWORDS
// DO_NOT_SUBMIT(mwhittaker): Document more.
struct ConnectionConfig {
  std::string host;
  std::uint16_t port;
  std::string user;
  std::string password;
  std::string dbname;

  // DO_NOT_SUBMIT(mwhittaker): Document more.
  std::string ToString() const;
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_CONNECTION_CONFIG_H_
