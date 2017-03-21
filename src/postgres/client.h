#ifndef POSTGRES_CLIENT_H_
#define POSTGRES_CLIENT_H_

#include <cstddef>
#include <cstdint>

#include <string>
#include <vector>

namespace fluent {
namespace postgres {

class Client {
 public:
  // DO_NOT_SUBMIT(mwhittaker): Document.
  virtual void Init(const std::string& name) = 0;

  // DO_NOT_SUBMIT(mwhittaker): Document.
  virtual void AddCollection(const std::string& collection_name,
                             const std::vector<std::string>& types) = 0;

  // DO_NOT_SUBMIT(mwhittaker): Document.
  template <typename RA>
  void AddRule(std::size_t rule_number, const RA& ra) {
    AddRule(rule_number, ra.ToDebugString());
  }

  // DO_NOT_SUBMIT(mwhittaker): Document.
  virtual void InsertTuple(const std::string& collection_name, std::size_t hash,
                           int time_inserted,
                           const std::vector<std::string>& values) = 0;

  virtual void DeleteTuple(const std::string& collection_name, std::size_t hash,
                           int time_deleted) = 0;

 private:
  virtual void AddRule(std::size_t rule_number, const std::string& rule) = 0;
};

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_CLIENT_H_
