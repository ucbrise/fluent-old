#include "collections/collection_util.h"

#include <string>

#include "glog/logging.h"

namespace fluent {

std::string CollectionTypeToString(CollectionType type) {
  switch (type) {
    case CollectionType::TABLE:
      return "Table";
    case CollectionType::SCRATCH:
      return "Scratch";
    case CollectionType::CHANNEL:
      return "Channel";
    case CollectionType::STDIN:
      return "Stdin";
    case CollectionType::STDOUT:
      return "Stdout";
    case CollectionType::PERIODIC:
      return "Periodic";
    default: {
      CHECK(false) << "Unreachable code.";
      return "";
    }
  }
}

}  // namespace fluent
