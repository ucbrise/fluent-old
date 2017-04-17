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

// This file was taken and modified from https://goo.gl/HCMvhq.

#ifndef COMMON_STATUS_OR_H_
#define COMMON_STATUS_OR_H_

#include "glog/logging.h"

#include "common/status.h"

namespace fluent {

// A StatusOr holds a Status (in the case of an error), or a value T.
template <typename T>
class StatusOr {
 public:
  // Has status UNKNOWN.
  inline StatusOr();

  // Builds from a non-OK status. Crashes if an OK status is specified.
  inline StatusOr(const Status& status);  // NOLINT

  // Builds from the specified value.
  inline StatusOr(const T& value);  // NOLINT

  // Copy constructor.
  inline StatusOr(const StatusOr& other);

  // Conversion copy constructor, T must be copy constructible from U.
  template <typename U>
  inline StatusOr(const StatusOr<U>& other);

  // Move constructor.
  inline StatusOr(StatusOr&& other);

  // Conversion move constructor, T must be move constructible from U.
  template <typename U>
  inline StatusOr(StatusOr<U>&& other);

  // Copy assignment operator.
  inline const StatusOr& operator=(const StatusOr& other);

  // Conversion copy assignment operator, T must be assignable from U.
  template <typename U>
  inline const StatusOr& operator=(const StatusOr<U>& other);

  // Move assignment operator.
  inline const StatusOr& operator=(StatusOr&& other);

  // Conversion move assignment operator, T must be assignable from U.
  template <typename U>
  inline const StatusOr& operator=(StatusOr<U>&& other);

  // Accessors.
  inline const Status& status() const { return status_; }

  // Shorthand for status().ok().
  inline bool ok() const { return status_.ok(); }

  // Returns value or crashes if ok() is false.
  inline const T& ValueOrDie() const {
    CHECK(ok()) << "Attempting to fetch value of non-OK StatusOr";
    return value_;
  }

  inline T& ValueOrDie() {
    CHECK(ok()) << "Attempting to fetch value of non-OK StatusOr";
    return value_;
  }

  inline T ConsumeValueOrDie() { return std::move(ValueOrDie()); }

  template <typename U>
  friend class StatusOr;

 private:
  Status status_;
  T value_;
};

// Implementation.

template <typename T>
inline StatusOr<T>::StatusOr() : status_(ErrorCode::UNKNOWN, "") {}

template <typename T>
inline StatusOr<T>::StatusOr(const Status& status) : status_(status) {
  CHECK(!status.ok()) << "Status::OK is not a valid argument to StatusOr";
}

template <typename T>
inline StatusOr<T>::StatusOr(const T& value) : value_(value) {}

template <typename T>
inline StatusOr<T>::StatusOr(const StatusOr& other)
    : status_(other.status_), value_(other.value_) {}

template <typename T>
template <typename U>
inline StatusOr<T>::StatusOr(const StatusOr<U>& other)
    : status_(other.status_), value_(other.value_) {}

template <typename T>
inline StatusOr<T>::StatusOr(StatusOr&& other)
    : status_(std::move(other.status_)), value_(std::move(other.value_)) {}

template <typename T>
template <typename U>
inline StatusOr<T>::StatusOr(StatusOr<U>&& other)
    : status_(std::move(other.status_)), value_(std::move(other.value_)) {}

template <typename T>
inline const StatusOr<T>& StatusOr<T>::operator=(const StatusOr& other) {
  status_ = other.status_;
  if (status_.ok()) {
    value_ = other.value_;
  }
  return *this;
}

template <typename T>
template <typename U>
inline const StatusOr<T>& StatusOr<T>::operator=(const StatusOr<U>& other) {
  status_ = other.status_;
  if (status_.ok()) {
    value_ = other.value_;
  }
  return *this;
}

template <typename T>
inline const StatusOr<T>& StatusOr<T>::operator=(StatusOr&& other) {
  status_ = std::move(other.status_);
  if (status_.ok()) {
    value_ = std::move(other.value_);
  }
  return *this;
}

template <typename T>
template <typename U>
inline const StatusOr<T>& StatusOr<T>::operator=(StatusOr<U>&& other) {
  status_ = std::move(other.status_);
  if (status_.ok()) {
    value_ = std::move(other.value_);
  }
  return *this;
}

}  // namespace fluent

#endif  // COMMON_STATUS_OR_H_
