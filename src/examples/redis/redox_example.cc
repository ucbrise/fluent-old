#include <iostream>

#include "glog/logging.h"
#include "redox.hpp"

#include "examples/redis/wrappers.h"

int main(int, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  // Connect to redis server.
  redox::Redox rdx;
  CHECK(rdx.connect("localhost", 6379))
      << "Could not connect to localhost:6379.";

  // Issue commands.
  CommandWrapper<std::string> set{
      rdx.commandSync<std::string>({"SET", "x", "4"})};
  CHECK(set.cmd.ok());
  std::cout << "SET: " << set.cmd.reply() << std::endl;

  CommandWrapper<int> append{rdx.commandSync<int>({"APPEND", "x", "2"})};
  CHECK(append.cmd.ok());
  std::cout << "APPEND: " << append.cmd.reply() << std::endl;

  CommandWrapper<std::string> get{rdx.commandSync<std::string>({"GET", "x"})};
  CHECK(get.cmd.ok());
  std::cout << "GET: " << get.cmd.reply() << std::endl;

  CommandWrapper<int> incr{rdx.commandSync<int>({"INCR", "x"})};
  CHECK(incr.cmd.ok());
  std::cout << "INCR: " << incr.cmd.reply() << std::endl;

  CommandWrapper<int> decr{rdx.commandSync<int>({"DECR", "x"})};
  CHECK(decr.cmd.ok());
  std::cout << "DECR: " << decr.cmd.reply() << std::endl;

  CommandWrapper<int> incrby{rdx.commandSync<int>({"INCRBY", "x", "100"})};
  CHECK(incrby.cmd.ok());
  std::cout << "INCRBY: " << incrby.cmd.reply() << std::endl;

  CommandWrapper<int> decrby{rdx.commandSync<int>({"DECRBY", "x", "100"})};
  CHECK(decrby.cmd.ok());
  std::cout << "DECRBY: " << decrby.cmd.reply() << std::endl;

  CommandWrapper<int> strlen{rdx.commandSync<int>({"STRLEN", "x"})};
  CHECK(strlen.cmd.ok());
  std::cout << "STRLEN: " << strlen.cmd.reply() << std::endl;

  CommandWrapper<int> del{rdx.commandSync<int>({"DEL", "x"})};
  CHECK(del.cmd.ok());
  std::cout << "DEL: " << del.cmd.reply() << std::endl;
}
