#ifndef EXAMPLES_PRIMALITY_PRIMALITY_H_
#define EXAMPLES_PRIMALITY_PRIMALITY_H_

template <typename FluentBuilder>
auto AddPrimalityApi(FluentBuilder f) {
  return std::move(f)
      .template channel<std::string, std::string, std::int64_t, int>(
          "is_prime_request", {{"dst_addr", "src_addr", "id", "x"}})
      .template channel<std::string, std::int64_t, bool>(
          "is_prime_response", {{"addr", "id", "is_prime"}});
}

#endif  // EXAMPLES_PRIMALITY_PRIMALITY_H_
