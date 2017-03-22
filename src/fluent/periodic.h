#ifndef FLUENT_PERIODIC_H_
#define FLUENT_PERIODIC_H_

#include <cstddef>

#include <chrono>
#include <set>
#include <string>
#include <tuple>
#include <utility>

#include "common/macros.h"
#include "common/type_traits.h"
#include "ra/iterable.h"

namespace fluent {

// A Periodic is a two-column collection where the first column is a unique id
// of type `id` and the second column is a time point of type `time`. It looks
// something like this:
//
//   +----+------------+
//   | id | time       |
//   +----+------------+
//   | 09 | 1488762221 |
//   +----+------------+
//   | 10 | 1488762226 |
//   +----+------------+
//   | 11 | 1488762236 |
//   +----+------------+
//
// You cannot write into a Periodic. Instead, Periodics are constructed with a
// period `period` (e.g. 1 second). Then, every `period` (e.g. every 1 second),
// a new tuple is inserted into the table with a unique id and the current
// time. After every tick, the Periodic is cleared.
class Periodic {
 public:
  using id = std::size_t;
  using clock = std::chrono::system_clock;
  using time = std::chrono::time_point<clock>;
  using period = std::chrono::milliseconds;

  Periodic(std::string name, period period)
      : name_(std::move(name)), period_(std::move(period)), id_(0) {}
  Periodic(Periodic&&) = default;
  Periodic& operator=(Periodic&&) = default;
  DISALLOW_COPY_AND_ASSIGN(Periodic);

  const std::string& Name() const { return name_; }

  const period& Period() const { return period_; }

  const std::set<std::tuple<id, time>>& Get() const { return tocks_; }

  ra::Iterable<std::set<std::tuple<id, time>>> Iterable() const {
    return ra::make_iterable(&tocks_);
  }

  void Tock() {
    tocks_.insert(
        std::tuple<id, time>(id_++, std::chrono::system_clock::now()));
  }

  void Tick() { tocks_.clear(); }

 private:
  const std::string name_;
  const std::chrono::milliseconds period_;
  id id_;
  std::set<std::tuple<id, time>> tocks_;
};

}  // namespace fluent

#endif  // FLUENT_PERIODIC_H_
