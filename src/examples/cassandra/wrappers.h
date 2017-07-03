#ifndef EXAMPLES_CASSANDRA_WRAPPERS_H_
#define EXAMPLES_CASSANDRA_WRAPPERS_H_

#include <memory>

#include "cassandra.h"

// Datastax's C++ Cassandra driver has you manually allocate and free various
// objects. These wrappers use RAII to automatically free the resources for
// you.

// CassCluster
struct CassClusterDeleter {
  void operator()(CassCluster* cluster) { cass_cluster_free(cluster); }
};
using CassClusterWrapper = std::unique_ptr<CassCluster, CassClusterDeleter>;

// CassSession
struct CassSessionDeleter {
  void operator()(CassSession* cluster) { cass_session_free(cluster); }
};
using CassSessionWrapper = std::unique_ptr<CassSession, CassSessionDeleter>;

// CassFuture
struct CassFutureDeleter {
  void operator()(CassFuture* cluster) { cass_future_free(cluster); }
};
using CassFutureWrapper = std::unique_ptr<CassFuture, CassFutureDeleter>;

// CassStatement
struct CassStatementDeleter {
  void operator()(CassStatement* cluster) { cass_statement_free(cluster); }
};
using CassStatementWrapper =
    std::unique_ptr<CassStatement, CassStatementDeleter>;

// CassResult
struct CassResultDeleter {
  void operator()(const CassResult* cluster) { cass_result_free(cluster); }
};
using CassResultWrapper = std::unique_ptr<const CassResult, CassResultDeleter>;

#endif  // EXAMPLES_CASSANDRA_WRAPPERS_H_
