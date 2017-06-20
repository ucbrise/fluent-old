#ifndef EXAMPLES_FILE_SYSTEM_FILE_SYSTEM_H_
#define EXAMPLES_FILE_SYSTEM_FILE_SYSTEM_H_

#include <cstdint>

using write_req_tuple =
    std::tuple<std::string, std::string, std::int64_t, int, std::string>;
using write_resp_tuple =  //
    std::tuple<std::string, std::int64_t>;
using read_req_tuple =
    std::tuple<std::string, std::string, std::int64_t, int, int>;
using read_resp_tuple =  //
    std::tuple<std::string, std::int64_t, std::string>;

template <typename FluentBuilder>
auto AddFileSystemApi(FluentBuilder f) {
  using std::string;
  return std::move(f)
      .template channel<string, string, std::int64_t, int, string>(  //
          "write_request",                                           //
          {{"dst_addr", "src_addr", "id", "start", "data"}})
      .template channel<string, std::int64_t>(  //
          "write_response",                     //
          {{"addr", "id"}})
      .template channel<string, string, std::int64_t, int, int>(  //
          "read_request",                                         //
          {{"dst_addr", "src_addr", "id", "start", "stop"}})
      .template channel<string, std::int64_t, string>(  //
          "read_response",                              //
          {{"addr", "id", "data"}});
}

#endif  // EXAMPLES_FILE_SYSTEM_FILE_SYSTEM_H_
