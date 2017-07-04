#ifndef EXAMPLES_CASSANDRA_API_H_
#define EXAMPLES_CASSANDRA_API_H_

#include <cstddef>

#include <string>
#include <tuple>

using set_request_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string, std::int32_t>;
using set_response_tuple = std::tuple<  //
    std::string, std::int64_t>;
using get_request_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string>;
using get_response_tuple = std::tuple<  //
    std::string, std::int64_t, std::int32_t, std::int64_t>;

template <typename FluentBuilder>
auto AddKvsApi(FluentBuilder f) {
  using string = std::string;
  return std::move(f)
      .template channel<string, string, std::int64_t, string, std::int32_t>(
          "set_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
      .template channel<string, std::int64_t>(  //
          "set_response", {{"addr", "id"}})
      .template channel<string, string, std::int64_t, string>(
          "get_request", {{"dst_addr", "src_addr", "id", "key"}})
      .template channel<string, std::int64_t, std::int32_t, std::int64_t>(
          "get_response", {{"addr", "id", "value", "reply_id"}});
}

#endif  // EXAMPLES_CASSANDRA_API_H_
