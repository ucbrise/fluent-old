/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef GRPC_INTERNAL_COMPILER_PYTHON_GENERATOR_H
#define GRPC_INTERNAL_COMPILER_PYTHON_GENERATOR_H

#include <utility>

#include "shim_gen/grpc/config.h"
#include "shim_gen/grpc/schema_interface.h"

namespace fluent_python_generator {

// Data pertaining to configuration of the generator with respect to anything
// that may be used internally at Google.
struct GeneratorConfiguration {
  GeneratorConfiguration();
  std::string grpc_package_root;
  // TODO(https://github.com/grpc/grpc/issues/8622): Drop this.
  std::string beta_package_root;
  // TODO(https://github.com/google/protobuf/issues/888): Drop this.
  std::string import_prefix;
};

class PythonGrpcGenerator : public google::protobuf::compiler::CodeGenerator {
 public:
  PythonGrpcGenerator(const GeneratorConfiguration& config);
  ~PythonGrpcGenerator();

  bool Generate(const google::protobuf::FileDescriptor* file,
                const std::string& parameter,
                google::protobuf::compiler::GeneratorContext* context,
                std::string* error) const;

 private:
  GeneratorConfiguration config_;
};

}  // namespace fluent_python_generator

#endif  // GRPC_INTERNAL_COMPILER_PYTHON_GENERATOR_H
