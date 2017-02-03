#ifndef FLUENT_SCRATCH_H_
#define FLUENT_SCRATCH_H_

#include "fluent/collection.h"

namespace fluent {

// A Scratch (or temporary relation). The contents of a Scratch are cleared
// between timesteps.
template <typename... Ts>
class Scratch : public Collection<Ts...> {
 public:
  explicit Scratch(const std::string& name) : Collection<Ts...>(name) {}

  void Add(const std::tuple<Ts...>& t) override {
    this->MutableGet().insert(t);
  }

  void Tick() override { this->MutableGet().clear(); }
};

}  // namespace fluent

#endif  // FLUENT_SCRATCH_H_
