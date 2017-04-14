#ifndef FLUENT_STDIN_H_
#define FLUENT_STDIN_H_

#include <algorithm>
#include <array>
#include <iostream>
#include <set>
#include <string>
#include <tuple>
#include <utility>

#include "range/v3/all.hpp"
#include "zmq.hpp"

#include "common/macros.h"

namespace fluent {

class Stdin {
 public:
  Stdin() {}
  DISALLOW_COPY_AND_ASSIGN(Stdin);

  const std::string& Name() const {
    static const std::string name = "stdin";
    return name;
  }

  const std::array<std::string, 1>& ColumnNames() const {
    static std::array<std::string, 1> column_names{{"stdin"}};
    return column_names;
  }

  ra::Iterable<std::set<std::tuple<std::string>>> Iterable() const {
    return ra::make_iterable(Name(), &lines_);
  }

  zmq::pollitem_t Pollitem() {
    return {/* socket */ nullptr, /* fd */ 0, /* events */ ZMQ_POLLIN,
            /* revents */ 0};
  }

  std::tuple<std::string> GetLine() {
    std::string line;
    std::getline(std::cin, line);
    std::tuple<std::string> t(line);
    lines_.insert(t);
    return t;
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
