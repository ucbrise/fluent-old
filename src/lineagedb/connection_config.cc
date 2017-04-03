#include "lineagedb/connection_config.h"

namespace fluent {
namespace lineagedb {

std::string ConnectionConfig::ToString() const {
  return                                 //
      "host=" + host +                   //
      " port=" + std::to_string(port) +  //
      " user=" + user +                  //
      " password=" + password +          //
      " dbname=" + dbname;
}

}  // namespace lineagedb
}  // namespace fluent
