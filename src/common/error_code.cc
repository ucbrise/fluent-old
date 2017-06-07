#include "common/error_code.h"

#include "glog/logging.h"

namespace fluent {

std::string ErrorCodeToString(ErrorCode error) {
  switch (error) {
    case ErrorCode::OK:
      return "OK";
    case ErrorCode::CANCELLED:
      return "CANCELLED";
    case ErrorCode::UNKNOWN:
      return "UNKNOWN";
    case ErrorCode::INVALID_ARGUMENT:
      return "INVALID_ARGUMENT";
    case ErrorCode::DEADLINE_EXCEEDED:
      return "DEADLINE_EXCEEDED";
    case ErrorCode::NOT_FOUND:
      return "NOT_FOUND";
    case ErrorCode::ALREADY_EXISTS:
      return "ALREADY_EXISTS";
    case ErrorCode::PERMISSION_DENIED:
      return "PERMISSION_DENIED";
    case ErrorCode::RESOURCE_EXHAUSTED:
      return "RESOURCE_EXHAUSTED";
    case ErrorCode::FAILED_PRECONDITION:
      return "FAILED_PRECONDITION";
    case ErrorCode::ABORTED:
      return "ABORTED";
    case ErrorCode::OUT_OF_RANGE:
      return "OUT_OF_RANGE";
    case ErrorCode::UNIMPLEMENTED:
      return "UNIMPLEMENTED";
    case ErrorCode::INTERNAL:
      return "INTERNAL";
    case ErrorCode::UNAVAILABLE:
      return "UNAVAILABLE";
    case ErrorCode::DATA_LOSS:
      return "DATA_LOSS";
    default: {
      CHECK(false) << "Unreachable code.";
      return "";
    }
  }
}

}  // namespace fluent
