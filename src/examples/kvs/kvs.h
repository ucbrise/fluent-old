#ifndef EXAMPLES_KVS_KVS_H_
#define EXAMPLES_KVS_KVS_H_

template <typename FluentBuilder>
auto AddKvsApi(FluentBuilder f) {
  using string = std::string;
  return std::move(f)
      .template channel<string, string, std::int64_t, string, string>(
          "set_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
      .template channel<string, std::int64_t>(  //
          "set_response", {{"addr", "id"}})
      .template channel<string, string, std::int64_t, string>(
          "get_request", {{"dst_addr", "src_addr", "id", "key"}})
      .template channel<string, std::int64_t, string>(
          "get_response", {{"addr", "id", "value"}});
}

#endif  // EXAMPLES_KVS_KVS_H_
