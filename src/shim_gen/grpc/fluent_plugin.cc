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

// Generates cpp gRPC service interface out of Protobuf IDL.
//

#include <memory>
#include <sstream>

#include "common/macros.h"
#include "shim_gen/grpc/config.h"
#include "shim_gen/grpc/cpp_generator.h"
#include "shim_gen/grpc/generator_helpers.h"
#include "shim_gen/grpc/protobuf_plugin.h"

class GrpcFluentShimGenerator
    : public google::protobuf::compiler::CodeGenerator {
 public:
  GrpcFluentShimGenerator() {}
  virtual ~GrpcFluentShimGenerator() {}

  virtual bool Generate(const google::protobuf::FileDescriptor* file,
                        const std::string& parameter,
                        google::protobuf::compiler::GeneratorContext* context,
                        std::string* error) const {
    UNUSED(parameter);
    UNUSED(error);

    ProtoBufFile pbfile(file);
    if (file->service_count() < 1) {
      *error = "No services found.";
      return false;
    }
    if (file->service_count() > 1) {
      *error = "The fluent plugin only supports one service per file.";
      return false;
    }

    fluent_cpp_generator::Parameters generator_parameters;
    std::string header_code =
        fluent_cpp_generator::GetPrologue(&pbfile, generator_parameters) +
        fluent_cpp_generator::GetIncludes(&pbfile, generator_parameters) +
        fluent_cpp_generator::GetClientClass(&pbfile, generator_parameters) +
        fluent_cpp_generator::GetApiFunction(&pbfile, generator_parameters) +
        fluent_cpp_generator::GetFluentFunction(&pbfile, generator_parameters) +
        fluent_cpp_generator::GetEpilogue(&pbfile, generator_parameters);
    std::string file_name = fluent_generator::StripProto(file->name());
    std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> header_output(
        context->Open(file_name + ".h"));
    google::protobuf::io::CodedOutputStream header_coded_out(
        header_output.get());
    header_coded_out.WriteRaw(header_code.data(), header_code.size());

    return true;
  }

 private:
  // Insert the given code into the given file at the given insertion point.
  void Insert(google::protobuf::compiler::GeneratorContext* context,
              const std::string& filename, const std::string& insertion_point,
              const std::string& code) const {
    std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
        context->OpenForInsert(filename, insertion_point));
    google::protobuf::io::CodedOutputStream coded_out(output.get());
    coded_out.WriteRaw(code.data(), code.size());
  }
};

int main(int argc, char* argv[]) {
  GrpcFluentShimGenerator generator;
  return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
