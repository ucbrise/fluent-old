#ifndef EXAMPLES_CASSANDRA_WORKLOADS_H_
#define EXAMPLES_CASSANDRA_WORKLOADS_H_

#include <random>
#include <string>

enum class Workload { UNIFORM, ZIPFIAN };

std::string WorkloadToString(const Workload& workload);

Workload StringToWorkload(const std::string& s);

std::discrete_distribution<int> WorkloadToDistribution(const Workload& workload,
                                                       const int num_keys);

#endif  //  EXAMPLES_CASSANDRA_WORKLOADS_H_
