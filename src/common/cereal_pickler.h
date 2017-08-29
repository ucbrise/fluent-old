#ifndef COMMON_CEREAL_PICKLER_H_
#define COMMON_CEREAL_PICKLER_H_

#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "glog/logging.h"

namespace fluent {
namespace common {

// A pickler (synonymously serializer or marshaller) is a struct which converts
// C++ objects to and from strings. The CerealPickler is a pickler implemented
// with the Cereal library [1].
//
// [1]: http://uscilab.github.io/cereal/
template <typename T>
struct CerealPickler {
  std::string Dump(const T& x) {
    std::stringstream ss;
    {
      // Scope the oarchive, so it's destructor can finalize the serialization.
      cereal::BinaryOutputArchive oarchive(ss);
      oarchive(x);
    }
    return ss.str();
  }

  T Load(const std::string& s) {
    std::stringstream ss(s);
    T x;
    {
      // Scope the iarchive, so it's destructor can finalize the serialization.
      cereal::BinaryInputArchive iarchive(ss);
      iarchive(x);
    }
    return x;
  }
};

}  // namespace common
}  // namespace fluent

#endif  // COMMON_CEREAL_PICKLER_H_
