// Copyright 2013 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file was taken and modified from https://goo.gl/3I9uYZ.

#ifndef COMMON_STATUS_H_
#define COMMON_STATUS_H_

#include <string>

#include "common/error_code.h"

namespace fluent {
namespace common {

// A Status is a combination of an error code and a string message (for non-OK
// error codes).
class Status {
 public:
  // Creates an OK status
  Status();

  // Make a Status from the specified error and message.
  Status(ErrorCode error, std::string error_message);

  // Default copy and move constructors and move assignment operators.
  Status(const Status& other) = default;
  Status& operator=(const Status& other) = default;
  Status(Status&& other) = default;
  Status& operator=(Status&& other) = default;

  // Some pre-defined Status objects
  static const Status& OK;  // Identical to 0-arg constructor
  static const Status& CANCELLED;
  static const Status& UNKNOWN;

  // Accessors
  bool ok() const { return code_ == ErrorCode::OK; }
  ErrorCode error_code() const { return code_; }
  const ::std::string& error_message() const { return message_; }

  bool operator==(const Status& x) const;
  bool operator!=(const Status& x) const;

  // NoOp
  void IgnoreError() const {}

  ::std::string ToString() const;

 private:
  ErrorCode code_;
  ::std::string message_;
};

inline bool Status::operator==(const Status& other) const {
  return (this->code_ == other.code_) && (this->message_ == other.message_);
}

inline bool Status::operator!=(const Status& other) const {
  return !(*this == other);
}

extern ::std::ostream& operator<<(::std::ostream& os, const Status& other);

}  // namespace common
}  // namespace fluent

#endif  // COMMON_STATUS_H_
