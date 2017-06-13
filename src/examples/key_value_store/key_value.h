#ifndef EXAMPLES_KEY_VALUE_STORE_KEY_VALUE_H_
#define EXAMPLES_KEY_VALUE_STORE_KEY_VALUE_H_

template <typename FluentBuilder>
auto AddKeyValueApi(FluentBuilder f) {
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

#endif  // EXAMPLES_KEY_VALUE_STORE_KEY_VALUE_H_
