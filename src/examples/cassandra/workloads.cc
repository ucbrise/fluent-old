#include "examples/cassandra/workloads.h"

#include <stdexcept>

#include "glog/logging.h"

#include "common/rand_util.h"

std::string WorkloadToString(const Workload& workload) {
  switch (workload) {
    case Workload::UNIFORM: {
      return "UNIFORM";
    }
    case Workload::ZIPFIAN: {
      return "ZIPFIAN";
    }
    default: {
      CHECK(false) << "Unreachable code.";
      throw std::runtime_error("Unreachable");
    }
  }
}

Workload StringToWorkload(const std::string& s) {
  if (s == WorkloadToString(Workload::UNIFORM)) {
    return Workload::UNIFORM;
  } else if (s == WorkloadToString(Workload::ZIPFIAN)) {
    return Workload::ZIPFIAN;
  } else {
    CHECK(false) << "Invalid workload " << s;
    throw std::runtime_error("Unreachable");
  }
}

std::discrete_distribution<int> WorkloadToDistribution(const Workload& workload,
                                                       const int num_keys) {
  switch (workload) {
    case Workload::UNIFORM: {
      std::vector<double> weights(num_keys, 1.0);
      return std::discrete_distribution<int>(weights.begin(), weights.end());
    }
    case Workload::ZIPFIAN: {
      return fluent::ZipfDistribution(num_keys, 1.0);
    }
    default: {
      CHECK(false) << "Unreachable code.";
      throw std::runtime_error("Unreachable");
    }
  }
}
