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

#include <ostream>

#include "fmt/format.h"

#include "common/status.h"

namespace fluent {
namespace common {

namespace {

const Status &GetOk() {
  static const Status status;
  return status;
}

const Status &GetCancelled() {
  static const Status status(ErrorCode::CANCELLED, "");
  return status;
}

const Status &GetUnknown() {
  static const Status status(ErrorCode::UNKNOWN, "");
  return status;
}

}  // namespace

Status::Status() : code_(ErrorCode::OK), message_("") {}

Status::Status(ErrorCode error, std::string error_message)
    : code_(error), message_(std::move(error_message)) {
  if (code_ == ErrorCode::OK) {
    message_.clear();
  }
}

const Status &Status::OK = GetOk();
const Status &Status::CANCELLED = GetCancelled();
const Status &Status::UNKNOWN = GetUnknown();

std::string Status::ToString() const {
  if (code_ == ErrorCode::OK) {
    return "OK";
  }

  return fmt::format("{}: {}", ErrorCodeToString(code_), message_);
}

extern std::ostream &operator<<(std::ostream &os, const Status &other) {
  os << other.ToString();
  return os;
}

}  // namespace common
}  // namespace fluent
