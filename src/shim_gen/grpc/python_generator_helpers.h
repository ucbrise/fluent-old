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

#ifndef GRPC_INTERNAL_COMPILER_PYTHON_GENERATOR_HELPERS_H
#define GRPC_INTERNAL_COMPILER_PYTHON_GENERATOR_HELPERS_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "shim_gen/grpc/config.h"
#include "shim_gen/grpc/generator_helpers.h"
#include "shim_gen/grpc/python_generator.h"
#include "shim_gen/grpc/python_private_generator.h"

using std::vector;
using fluent_generator::StringReplace;
using fluent_generator::StripProto;
using google::protobuf::Descriptor;
using google::protobuf::FileDescriptor;
using google::protobuf::MethodDescriptor;
using google::protobuf::ServiceDescriptor;
using google::protobuf::compiler::GeneratorContext;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::io::Printer;
using google::protobuf::io::StringOutputStream;
using google::protobuf::io::ZeroCopyOutputStream;

namespace fluent_python_generator {

namespace {

typedef vector<const Descriptor*> DescriptorVector;
typedef vector<std::string> StringVector;

// TODO(https://github.com/google/protobuf/issues/888):
// Export `ModuleName` from protobuf's
// `src/google/protobuf/compiler/python/python_generator.cc` file.
std::string ModuleName(const std::string& filename,
                        const std::string& import_prefix) {
  std::string basename = StripProto(filename);
  basename = StringReplace(basename, "-", "_");
  basename = StringReplace(basename, "/", ".");
  return import_prefix + basename + "_pb2";
}

// TODO(https://github.com/google/protobuf/issues/888):
// Export `ModuleAlias` from protobuf's
// `src/google/protobuf/compiler/python/python_generator.cc` file.
std::string ModuleAlias(const std::string& filename,
                         const std::string& import_prefix) {
  std::string module_name = ModuleName(filename, import_prefix);
  // We can't have dots in the module name, so we replace each with _dot_.
  // But that could lead to a collision between a.b and a_dot_b, so we also
  // duplicate each underscore.
  module_name = StringReplace(module_name, "_", "__");
  module_name = StringReplace(module_name, ".", "_dot_");
  return module_name;
}

bool GetModuleAndMessagePath(const Descriptor* type, std::string* out,
                             std::string generator_file_name,
                             bool generate_in_pb2_grpc,
                             std::string& import_prefix) {
  const Descriptor* path_elem_type = type;
  DescriptorVector message_path;
  do {
    message_path.push_back(path_elem_type);
    path_elem_type = path_elem_type->containing_type();
  } while (path_elem_type);  // implicit nullptr comparison; don't be explicit
  std::string file_name = type->file()->name();
  static const int proto_suffix_length = strlen(".proto");
  if (!(file_name.size() > static_cast<size_t>(proto_suffix_length) &&
        file_name.find_last_of(".proto") == file_name.size() - 1)) {
    return false;
  }

  std::string module;
  if (generator_file_name != file_name || generate_in_pb2_grpc) {
    module = ModuleAlias(file_name, import_prefix) + ".";
  } else {
    module = "";
  }
  std::string message_type;
  for (DescriptorVector::reverse_iterator path_iter = message_path.rbegin();
       path_iter != message_path.rend(); ++path_iter) {
    message_type += (*path_iter)->name() + ".";
  }
  // no pop_back prior to C++11
  message_type.resize(message_type.size() - 1);
  *out = module + message_type;
  return true;
}

template <typename DescriptorType>
StringVector get_all_comments(const DescriptorType* descriptor) {
  StringVector comments;
  fluent_generator::GetComment(
      descriptor, fluent_generator::COMMENTTYPE_LEADING_DETACHED, &comments);
  fluent_generator::GetComment(descriptor, fluent_generator::COMMENTTYPE_LEADING,
                             &comments);
  fluent_generator::GetComment(descriptor, fluent_generator::COMMENTTYPE_TRAILING,
                             &comments);
  return comments;
}

}  // namespace

}  // namespace fluent_python_generator

#endif  // GRPC_INTERNAL_COMPILER_PYTHON_GENERATOR_HELPERS_H
