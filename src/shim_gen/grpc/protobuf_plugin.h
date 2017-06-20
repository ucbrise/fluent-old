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

#ifndef GRPC_INTERNAL_COMPILER_PROTOBUF_PLUGIN_H
#define GRPC_INTERNAL_COMPILER_PROTOBUF_PLUGIN_H

#include "shim_gen/grpc/config.h"
#include "shim_gen/grpc/cpp_generator_helpers.h"
#include "shim_gen/grpc/python_generator_helpers.h"
#include "shim_gen/grpc/python_private_generator.h"
#include "shim_gen/grpc/schema_interface.h"

#include <vector>

// Get leading or trailing comments in a string.
template <typename DescriptorType>
inline std::string GetCommentsHelper(const DescriptorType *desc, bool leading,
                                     const std::string &prefix) {
  return fluent_generator::GetPrefixedComments(desc, leading, prefix);
}

class ProtoBufMethod : public fluent_generator::Method {
 public:
  ProtoBufMethod(const google::protobuf::MethodDescriptor *method)
      : method_(method) {}

  std::string name() const { return method_->name(); }

  const google::protobuf::Descriptor *input_type() const {
    return method_->input_type();
  }
  const google::protobuf::Descriptor *output_type() const {
    return method_->output_type();
  }

  std::string input_type_name() const {
    return fluent_cpp_generator::ClassName(method_->input_type(), true);
  }
  std::string output_type_name() const {
    return fluent_cpp_generator::ClassName(method_->output_type(), true);
  }

  std::string get_input_type_name() const {
    return method_->input_type()->file()->name();
  }
  std::string get_output_type_name() const {
    return method_->output_type()->file()->name();
  }

  bool get_module_and_message_path_input(std::string *str,
                                         std::string generator_file_name,
                                         bool generate_in_pb2_grpc,
                                         std::string import_prefix) const {
    return fluent_python_generator::GetModuleAndMessagePath(
        method_->input_type(), str, generator_file_name, generate_in_pb2_grpc,
        import_prefix);
  }

  bool get_module_and_message_path_output(std::string *str,
                                          std::string generator_file_name,
                                          bool generate_in_pb2_grpc,
                                          std::string import_prefix) const {
    return fluent_python_generator::GetModuleAndMessagePath(
        method_->output_type(), str, generator_file_name, generate_in_pb2_grpc,
        import_prefix);
  }

  bool NoStreaming() const {
    return !method_->client_streaming() && !method_->server_streaming();
  }

  bool ClientStreaming() const { return method_->client_streaming(); }

  bool ServerStreaming() const { return method_->server_streaming(); }

  bool BidiStreaming() const {
    return method_->client_streaming() && method_->server_streaming();
  }

  std::string GetLeadingComments(const std::string prefix) const {
    return GetCommentsHelper(method_, true, prefix);
  }

  std::string GetTrailingComments(const std::string prefix) const {
    return GetCommentsHelper(method_, false, prefix);
  }

  vector<std::string> GetAllComments() const {
    return fluent_python_generator::get_all_comments(method_);
  }

 private:
  const google::protobuf::MethodDescriptor *method_;
};

class ProtoBufService : public fluent_generator::Service {
 public:
  ProtoBufService(const google::protobuf::ServiceDescriptor *service)
      : service_(service) {}

  std::string name() const { return service_->name(); }

  int method_count() const { return service_->method_count(); };
  std::unique_ptr<const fluent_generator::Method> method(int i) const {
    return std::unique_ptr<const fluent_generator::Method>(
        new ProtoBufMethod(service_->method(i)));
  };

  std::string GetLeadingComments(const std::string prefix) const {
    return GetCommentsHelper(service_, true, prefix);
  }

  std::string GetTrailingComments(const std::string prefix) const {
    return GetCommentsHelper(service_, false, prefix);
  }

  vector<std::string> GetAllComments() const {
    return fluent_python_generator::get_all_comments(service_);
  }

 private:
  const google::protobuf::ServiceDescriptor *service_;
};

class ProtoBufPrinter : public fluent_generator::Printer {
 public:
  ProtoBufPrinter(std::string *str)
      : output_stream_(str), printer_(&output_stream_, '$') {}

  void Print(const std::map<std::string, std::string> &vars,
             const char *string_template) {
    printer_.Print(vars, string_template);
  }

  void Print(const char *string) { printer_.Print(string); }
  void Indent() { printer_.Indent(); }
  void Outdent() { printer_.Outdent(); }

 private:
  google::protobuf::io::StringOutputStream output_stream_;
  google::protobuf::io::Printer printer_;
};

class ProtoBufFile : public fluent_generator::File {
 public:
  ProtoBufFile(const google::protobuf::FileDescriptor *file) : file_(file) {}

  std::string filename() const { return file_->name(); }
  std::string filename_without_ext() const {
    return fluent_generator::StripProto(filename());
  }

  std::string package() const { return file_->package(); }
  std::vector<std::string> package_parts() const {
    return fluent_generator::tokenize(package(), ".");
  }

  std::string additional_headers() const { return ""; }

  int service_count() const { return file_->service_count(); };
  std::unique_ptr<const fluent_generator::Service> service(int i) const {
    return std::unique_ptr<const fluent_generator::Service>(
        new ProtoBufService(file_->service(i)));
  }

  std::unique_ptr<fluent_generator::Printer> CreatePrinter(
      std::string *str) const {
    return std::unique_ptr<fluent_generator::Printer>(new ProtoBufPrinter(str));
  }

  std::string GetLeadingComments(const std::string prefix) const {
    return GetCommentsHelper(file_, true, prefix);
  }

  std::string GetTrailingComments(const std::string prefix) const {
    return GetCommentsHelper(file_, false, prefix);
  }

  vector<std::string> GetAllComments() const {
    return fluent_python_generator::get_all_comments(file_);
  }

 private:
  const google::protobuf::FileDescriptor *file_;
};

#endif  // GRPC_INTERNAL_COMPILER_PROTOBUF_PLUGIN_H
