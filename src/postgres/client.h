#ifndef POSTGRES_CLIENT_H_
#define POSTGRES_CLIENT_H_

#include <cstdint>

#include <string>

namespace fluent {
namespace postgres {

class Client {
 public:
  // DO_NOT_SUBMIT(mwhittaker): Document.
  virtual void Init(const std::string& name) = 0;

  // DO_NOT_SUBMIT(mwhittaker): Document.
  virtual void AddCollection(const std::string& collection_name) = 0;

  // DO_NOT_SUBMIT(mwhittaker): Document.
  template <typename RA>
  void AddRule(const RA& ra) {
    // DO_NOT_SUBMIT(mwhittaker): Invoke AddRule below.
    (void)ra;
  }

 private:
  virtual void AddRule(const std::string& rule) = 0;
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_CLIENT_H_
