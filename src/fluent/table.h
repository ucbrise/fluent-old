#ifndef FLUENT_TABLE_H_
#define FLUENT_TABLE_H_

#include "fluent/collection.h"

namespace fluent {

// A Table (or relation). The contents of a Table persist across ticks.
template <typename... Ts>
class Table : public Collection<Ts...> {
 public:
  explicit Table(const std::string& name) : Collection<Ts...>(name) {}

  void Add(const std::tuple<Ts...>& t) override {
    this->MutableGet().insert(t);
  }

  void Tick() override {}
};

}  // namespace fluent

#endif  // FLUENT_TABLE_H_
