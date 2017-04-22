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

#include "boost/variant.hpp"
#include "glog/logging.h"

#include "common/status.h"

namespace fluent {

// A StatusOr holds a Status (in the case of an error), or a value T.
// TODO(mwhittaker): Write unit tests for StatusOr.
template <typename T>
class StatusOr {
 public:
  // Has status UNKNOWN.
  inline StatusOr();

  // Builds from a non-OK status. Crashes if an OK status is specified.
  inline StatusOr(const Status& status);  // NOLINT

  // Copies from the specified value.
  inline StatusOr(const T& value);  // NOLINT

  // Moves from the specified value.
  inline StatusOr(T&& value);  // NOLINT

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
  inline const Status& status() const {
    if (variant_.which() == 0) {
      return boost::get<Status>(variant_);
    } else {
      return Status::OK;
    }
  }

  // Shorthand for status().ok().
  inline bool ok() const {
    if (variant_.which() == 1) {
      return true;
    } else {
      CHECK(!boost::get<Status>(variant_).ok());
      return false;
    }
  }

  // Returns value or crashes if ok() is false.
  inline const T& ValueOrDie() const {
    CHECK(ok()) << "Attempting to fetch value of non-OK StatusOr: " << status();
    return boost::get<T>(variant_);
  }

  inline T& ValueOrDie() {
    CHECK(ok()) << "Attempting to fetch value of non-OK StatusOr: " << status();
    return boost::get<T>(variant_);
  }

  inline T ConsumeValueOrDie() { return std::move(ValueOrDie()); }

  template <typename U>
  friend class StatusOr;

 private:
  boost::variant<Status, T> variant_;
};

// Implementation.

template <typename T>
inline StatusOr<T>::StatusOr() : variant_(Status(ErrorCode::UNKNOWN, "")) {}

template <typename T>
inline StatusOr<T>::StatusOr(const Status& status) : variant_(status) {
  CHECK(!status.ok()) << "Status::OK is not a valid argument to StatusOr";
}

template <typename T>
inline StatusOr<T>::StatusOr(const T& value) : variant_(value) {}

template <typename T>
inline StatusOr<T>::StatusOr(T&& value) : variant_(std::move(value)) {}

template <typename T>
inline StatusOr<T>::StatusOr(const StatusOr& other)
    : variant_(other.variant_) {}

template <typename T>
template <typename U>
inline StatusOr<T>::StatusOr(const StatusOr<U>& other)
    : variant_(other.variant_) {}

template <typename T>
inline StatusOr<T>::StatusOr(StatusOr&& other)
    : variant_(std::move(other.variant_)) {}

template <typename T>
template <typename U>
inline StatusOr<T>::StatusOr(StatusOr<U>&& other)
    : variant_(std::move(other.variant_)) {}

template <typename T>
inline const StatusOr<T>& StatusOr<T>::operator=(const StatusOr& other) {
  variant_ = other.variant_;
  return *this;
}

template <typename T>
template <typename U>
inline const StatusOr<T>& StatusOr<T>::operator=(const StatusOr<U>& other) {
  variant_ = other.variant_;
  return *this;
}

template <typename T>
inline const StatusOr<T>& StatusOr<T>::operator=(StatusOr&& other) {
  variant_ = std::move(other.variant_);
  return *this;
}

template <typename T>
template <typename U>
inline const StatusOr<T>& StatusOr<T>::operator=(StatusOr<U>&& other) {
  variant_ = std::move(other.variant_);
  return *this;
}

}  // namespace fluent

#endif  // COMMON_STATUS_OR_H_
