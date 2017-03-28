#ifndef POSTGRES_MOCK_CONNECTION_H_
#define POSTGRES_MOCK_CONNECTION_H_

namespace fluent {
namespace postgres {

// A mock of the `pqxx::connection` class. See `InjectiblePqxxClient` for more
// information.
class MockConnection {
 public:
  MockConnection(const std::string&) {}
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_MOCK_CONNECTION_H_
