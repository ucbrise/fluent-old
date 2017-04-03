#ifndef LINEAGEDB_MOCK_CONNECTION_H_
#define LINEAGEDB_MOCK_CONNECTION_H_

namespace fluent {
namespace lineagedb {

// A mock of the `pqxx::connection` class. See `InjectiblePqxxClient` for more
// information.
class MockConnection {
 public:
  MockConnection(const std::string&) {}
};

}  // namespace lineagedb
}  // namespace fluent

#endif  // LINEAGEDB_MOCK_CONNECTION_H_
