#ifndef LINEAGEDB_MOCK_WORK_H_
#define LINEAGEDB_MOCK_WORK_H_

#include <string>

#include "lineagedb/mock_connection.h"

namespace fluent {
namespace lineagedb {

// A mock of the `pqxx::work` class. See `InjectiblePqxxClient` for more
// information.
class MockWork {
 public:
  MockWork(const MockConnection&, const std::string&) {}
  void exec(const std::string&) {}
  void commit() {}
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_MOCK_WORK_H_
