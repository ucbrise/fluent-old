#ifndef FLUENT_FLUENT_H_
#define FLUENT_FLUENT_H_

// Many C++ libraries combine all of their header files into a single header
// file to make life easier for library consumers. For example,
//
//   #include "grpc++/grpc++.h"
//   #include "range/v3/all.hpp"
//   #include "zmq.hpp"
//
// This is the "one header file to rule them all" for fluent. Include it in
// your fluent program and you should be good to go!
//
//   #include "fluent/fluent.h"
#include "common/status.h"
#include "common/status_or.h"
#include "fluent/fluent_builder.h"
#include "fluent/fluent_executor.h"
#include "fluent/infix.h"
#include "lineagedb/connection_config.h"
#include "lineagedb/noop_client.h"
#include "lineagedb/pqxx_client.h"
#include "ra/logical/all.h"

#endif  // FLUENT_FLUENT_H_
