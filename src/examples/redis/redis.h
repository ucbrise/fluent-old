#ifndef EXAMPLES_REDIS_REDIS_H_
#define EXAMPLES_REDIS_REDIS_H_

#include <cstddef>

#include <string>

using addr = std::string;
using id = std::int64_t;

using set_req_tuple = std::tuple<addr, addr, id, std::string, std::string>;
using set_resp_tuple = std::tuple<addr, id, std::string>;
using del_req_tuple = std::tuple<addr, addr, id, std::string>;
using del_resp_tuple = std::tuple<addr, id, int>;
using append_req_tuple = std::tuple<addr, addr, id, std::string, std::string>;
using append_resp_tuple = std::tuple<addr, id, int>;
using incr_req_tuple = std::tuple<addr, addr, id, std::string>;
using incr_resp_tuple = std::tuple<addr, id, int>;
using decr_req_tuple = std::tuple<addr, addr, id, std::string>;
using decr_resp_tuple = std::tuple<addr, id, int>;
using incrby_req_tuple = std::tuple<addr, addr, id, std::string, int>;
using incrby_resp_tuple = std::tuple<addr, id, int>;
using decrby_req_tuple = std::tuple<addr, addr, id, std::string, int>;
using decrby_resp_tuple = std::tuple<addr, id, int>;
using get_req_tuple = std::tuple<addr, addr, id, std::string>;
using get_resp_tuple = std::tuple<addr, id, std::string>;
using strlen_req_tuple = std::tuple<addr, addr, id, std::string>;
using strlen_resp_tuple = std::tuple<addr, id, int>;

template <typename FluentBuilder>
auto AddRedisApi(FluentBuilder f) {
  return std::move(f)
      // Overwriting commands.
      // SET (https://redis.io/commands/set)
      .template channel<addr, addr, id, std::string, std::string>(  //
          "set_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
      .template channel<addr, id, std::string>(  //
          "set_response", {{"addr", "id", "ret"}})
      // DEL (https://redis.io/commands/del)
      .template channel<addr, addr, id, std::string>(  //
          "del_request", {{"dst_addr", "src_addr", "id", "key"}})
      .template channel<addr, id, int>(  //
          "del_response", {{"addr", "id", "ret"}})

      // Modifying commands.
      // APPEND (https://redis.io/commands/append)
      .template channel<addr, addr, id, std::string, std::string>(  //
          "append_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
      .template channel<addr, id, int>(  //
          "append_response", {{"addr", "id", "ret"}})
      // INCR (https://redis.io/commands/incr)
      .template channel<addr, addr, id, std::string>(  //
          "incr_request", {{"dst_addr", "src_addr", "id", "key"}})
      .template channel<addr, id, int>(  //
          "incr_response", {{"addr", "id", "ret"}})
      // DECR (https://redis.io/commands/decr)
      .template channel<addr, addr, id, std::string>(  //
          "decr_request", {{"dst_addr", "src_addr", "id", "key"}})
      .template channel<addr, id, int>(  //
          "decr_response", {{"addr", "id", "ret"}})
      // INCRBY (https://redis.io/commands/incrby)
      .template channel<addr, addr, id, std::string, int>(
          "incrby_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
      .template channel<addr, id, int>(  //
          "incrby_response", {{"addr", "id", "ret"}})
      // DECRBY (https://redis.io/commands/decrby)
      .template channel<addr, addr, id, std::string, int>(  //
          "decrby_request", {{"dst_addr", "src_addr", "id", "key", "value"}})
      .template channel<addr, id, int>(  //
          "decrby_response", {{"addr", "id", "ret"}})

      // Reading commands.
      // GET (https://redis.io/commands/get)
      .template channel<addr, addr, id, std::string>(  //
          "get_request", {{"dst_addr", "src_addr", "id", "key"}})
      .template channel<addr, id, std::string>(  //
          "get_response", {{"addr", "id", "ret"}})
      // STRLEN (https://redis.io/commands/strlen)
      .template channel<addr, addr, id, std::string>(  //
          "strlen_request", {{"dst_addr", "src_addr", "id", "key"}})
      .template channel<addr, id, int>(  //
          "strlen_response", {{"addr", "id", "ret"}})

      ;
}

#endif  // EXAMPLES_REDIS_REDIS_H_
