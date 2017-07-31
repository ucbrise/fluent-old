#ifndef EXAMPLES_S3_API_BENCHMARK_H_
#define EXAMPLES_S3_API_BENCHMARK_H_

#include <cstddef>

#include <string>
#include <tuple>

using echo_req_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string, std::string,
    std::string>;
using echo_resp_tuple = std::tuple<  //
    std::string, std::int64_t, std::string>;
using rm_req_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string, std::string>;
using rm_resp_tuple = std::tuple<  //
    std::string, std::int64_t, std::string>;
using cat_req_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string, std::string>;
using cat_resp_tuple = std::tuple<  //
    std::string, std::int64_t, std::string, std::string>;

template <typename FluentBuilder>
auto AddS3Api(FluentBuilder f) {
  using str = std::string;
  return std::move(f)
      // Create object.
      .template channel<str, str, std::int64_t, str, str, str>(  //
          "echo_request",
          {{"dst_addr", "src_addr", "id", "bucket", "key", "part"}})
      .template channel<str, std::int64_t, str>(  //
          "echo_response", {{"addr", "id", "err"}})
      // Remove object.
      .template channel<str, str, std::int64_t, str, str>(  //
          "rm_request", {{"dst_addr", "src_addr", "id", "bucket", "key"}})
      .template channel<str, std::int64_t, str>(  //
          "rm_response", {{"addr", "id", "err"}})
      // Cat object.
      .template channel<str, str, std::int64_t, str, str>(  //
          "cat_request", {{"dst_addr", "src_addr", "id", "bucket", "key"}})
      .template channel<str, std::int64_t, str, str>(  //
          "cat_response", {{"addr", "id", "success", "err"}});
}
#endif  // EXAMPLES_S3_API_BENCHMARK_H_
