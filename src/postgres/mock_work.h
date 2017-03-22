#ifndef POSTGRES_MOCK_WORK_H_
#define POSTGRES_MOCK_WORK_H_

#include <string>

#include "postgres/mock_connection.h"

namespace fluent {
namespace postgres {

// DO_NOT_SUBMIT(mwhittaker): Document.
class MockWork {
 public:
  MockWork(const MockConnection&, const std::string&) {}
  void exec(const std::string&) {}
  void commit() {}
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_MOCK_WORK_H_
