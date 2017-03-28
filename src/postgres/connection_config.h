#ifndef POSTGRES_CONNECTION_CONFIG_H_
#define POSTGRES_CONNECTION_CONFIG_H_

#include <cstdint>

#include <string>

namespace fluent {
namespace postgres {

// PostgreSQL's libpq library and the libpqxx library use connection parameters
// to configure a connection to postgres [1]. This struct includes some of the
// more common connection parameters.
//
// [1]: http://bit.ly/2ncmbDL
struct ConnectionConfig {
  std::string host;
  std::uint16_t port;
  std::string user;
  std::string password;
  std::string dbname;

  // Convert a `ConnectionConfig` into a connection string understood by libpq
  // and libpqxx. See [1] for more information.
  //
  // [1]: http://bit.ly/2osDosy
  std::string ToString() const;
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_CONNECTION_CONFIG_H_
