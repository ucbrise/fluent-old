#ifndef EXAMPLES_S3_API_H_
#define EXAMPLES_S3_API_H_

#include <cstddef>

#include <string>
#include <tuple>

using mb_req_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string>;
using mb_resp_tuple = std::tuple<  //
    std::string, std::int64_t, std::string>;
using rb_req_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string>;
using rb_resp_tuple = std::tuple<  //
    std::string, std::int64_t, std::string>;
using echo_req_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string, std::string,
    std::string>;
using echo_resp_tuple = std::tuple<  //
    std::string, std::int64_t, std::string>;
using rm_req_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string, std::string>;
using rm_resp_tuple = std::tuple<  //
    std::string, std::int64_t, std::string>;
using ls_req_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string>;
using ls_resp_tuple = std::tuple<  //
    std::string, std::int64_t, std::vector<std::string>, std::string>;
using cat_req_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string, std::string>;
using cat_resp_tuple = std::tuple<  //
    std::string, std::int64_t, std::string, std::string>;
using cp_req_tuple = std::tuple<  //
    std::string, std::string, std::int64_t, std::string, std::string,
    std::string, std::string>;
using cp_resp_tuple = std::tuple<  //
    std::string, std::int64_t, std::string>;

template <typename FluentBuilder>
auto AddS3Api(FluentBuilder f) {
  using str = std::string;
  return std::move(f)
      // Make bucket.
      .template channel<str, str, std::int64_t, str>(  //
          "mb_request", {{"dst_addr", "src_addr", "id", "bucket"}})
      .template channel<str, std::int64_t, str>(  //
          "mb_response", {{"addr", "id", "err"}})
      // Remove bucket.
      .template channel<str, str, std::int64_t, str>(  //
          "rb_request", {{"dst_addr", "src_addr", "id", "bucket"}})
      .template channel<str, std::int64_t, str>(  //
          "rb_response", {{"addr", "id", "err"}})

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

      // List objects.
      .template channel<str, str, std::int64_t, str>(  //
          "ls_request", {{"dst_addr", "src_addr", "id", "bucket"}})
      .template channel<str, std::int64_t, std::vector<str>, str>(  //
          "ls_response", {{"addr", "id", "keys", "err"}})
      // Cat object.
      .template channel<str, str, std::int64_t, str, str>(  //
          "cat_request", {{"dst_addr", "src_addr", "id", "bucket", "key"}})
      .template channel<str, std::int64_t, str, str>(  //
          "cat_response", {{"addr", "id", "success", "err"}})
      // Copy object.
      .template channel<str, str, std::int64_t, str, str, str, str>(  //
          "cp_request",
          {{"dst_addr", "src_addr", "id", "src_bucket", "src_key", "dst_bucket",
            "dst_key"}})
      .template channel<str, std::int64_t, str>(  //
          "cp_response", {{"addr", "id", "err"}});
}
#endif  // EXAMPLES_S3_API_H_
