#ifndef EXAMPLES_BLACK_BOXES_PRIMALITY_H_
#define EXAMPLES_BLACK_BOXES_PRIMALITY_H_

#include <string>

template <typename FluentBuilder>
auto AddTwitterApi(FluentBuilder f) {
  return std::move(f)
      .template channel<std::string, std::string, std::string>(  //
          "tweet_req", {{"dst_addr", "src_addr", "tweet"}})
      .template channel<std::string, int>(  //
          "tweet_resp", {{"addr", "tweet_id"}})
      .template channel<std::string, std::string, int>(  //
          "fetch_req", {{"dst_addr", "src_addr", "tweet_id"}})
      .template channel<std::string, std::string>(  //
          "fetch_resp", {{"addr", "tweet"}})
      .template channel<std::string, int, std::string>(  //
          "update", {{"addr", "tweet_id", "tweet"}});
}

#endif  // EXAMPLES_BLACK_BOXES_PRIMALITY_H_
