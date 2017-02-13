#ifndef FLUENT_STDIN_H_
#define FLUENT_STDIN_H_

#include <iostream>
#include <set>
#include <string>
#include <tuple>
#include <utility>

#include "range/v3/all.hpp"
#include "zmq.hpp"

#include "common/macros.h"
#include "ra/ra_util.h"

namespace fluent {

class Stdin {
 public:
  Stdin() {}
  DISALLOW_COPY_AND_ASSIGN(Stdin);

  ra::Iterable<std::set<std::tuple<std::string>>> Iterable() const {
    return ra::make_iterable(&lines_);
  }

  zmq::pollitem_t Pollitem() {
    return {/* socket */ nullptr, /* fd */ 0, /* events */ ZMQ_POLLIN,
            /* revents */ 0};
  }

  void GetLine() {
    std::string line;
    std::getline(std::cin, line);
    lines_.insert(std::tuple<std::string>(line));
  }

  void Tick() { lines_.clear(); }

 private:
  std::set<std::tuple<std::string>> lines_;
};

}  // namespace fluent

#endif  // FLUENT_STDIN_H_
