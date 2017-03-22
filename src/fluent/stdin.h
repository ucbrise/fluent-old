#ifndef FLUENT_STDIN_H_
#define FLUENT_STDIN_H_

#include <algorithm>
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

  const std::string& Name() const {
    static const std::string name = "stdin";
    return name;
  }

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

  std::set<std::tuple<std::string>> Tick() {
    std::set<std::tuple<std::string>> lines;
    std::swap(lines_, lines);
    return lines;
  }

 private:
  std::set<std::tuple<std::string>> lines_;
};

}  // namespace fluent

#endif  // FLUENT_STDIN_H_
