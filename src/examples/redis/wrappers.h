#ifndef EXAMPLES_REDIS_WRAPPERS_H_
#define EXAMPLES_REDIS_WRAPPERS_H_

#include "redox.hpp"

template <typename ReplyT>
struct CommandWrapper {
  redox::Command<ReplyT>& cmd;
  ~CommandWrapper() { cmd.free(); }
};

#endif  // EXAMPLES_REDIS_WRAPPERS_H_
