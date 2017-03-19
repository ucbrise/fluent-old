#include "postgres/connection_config.h"

namespace fluent {
namespace postgres {

std::string ConnectionConfig::ToString() const {
  return                                 //
      "host=" + host +                   //
      " port=" + std::to_string(port) +  //
      " user=" + user +                  //
      " password=" + password +          //
      " dbname=" + dbname;
}

}  // namespace postgres
}  // namespace fluent
