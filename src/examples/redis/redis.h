#ifndef EXAMPLES_REDIS_REDIS_H_
#define EXAMPLES_REDIS_REDIS_H_

template <typename FluentBuilder>
auto AddRedisApi(FluentBuilder f) {
  using string = std::string;
  return std::move(f)
      .template channel<string, string, std::int64_t, string, string>(
          "set_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
      .template channel<string, std::int64_t, bool>(  //
          "set_response", {{"addr", "id", "success"}})
      .template channel<string, string, std::int64_t, string, string>(
          "append_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
      .template channel<string, std::int64_t, int>(  //
          "append_response", {{"addr", "id", "length"}})
      .template channel<string, string, std::int64_t, string>(
          "get_request", {{"dst_addr", "src_addr", "id", "key"}})
      .template channel<string, std::int64_t, string>(
          "get_response", {{"addr", "id", "value"}});
}

#endif  // EXAMPLES_REDIS_REDIS_H_
