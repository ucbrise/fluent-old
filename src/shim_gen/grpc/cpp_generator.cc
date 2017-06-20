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

#include <map>
#include <sstream>

#include "fmt/format.h"
#include "glog/logging.h"

#include "common/macros.h"
#include "common/string_util.h"
#include "shim_gen/grpc/cpp_generator.h"

namespace fluent_cpp_generator {

namespace {

#if 0
template <class T>
std::string as_string(T x) {
  std::ostringstream out;
  out << x;
  return out.str();
}

inline bool ClientOnlyStreaming(const fluent_generator::Method *method) {
  return method->ClientStreaming() && !method->ServerStreaming();
}

inline bool ServerOnlyStreaming(const fluent_generator::Method *method) {
  return !method->ClientStreaming() && method->ServerStreaming();
}
#endif

std::string FilenameIdentifier(const std::string &filename) {
  std::string result;
  for (unsigned i = 0; i < filename.size(); i++) {
    char c = filename[i];
    if (isalnum(c)) {
      result.push_back(c);
    } else {
      static char hex[] = "0123456789abcdef";
      result.push_back('_');
      result.push_back(hex[(c >> 4) & 0xf]);
      result.push_back(hex[c & 0xf]);
    }
  }
  return result;
}

std::vector<std::string> FieldTypes(const google::protobuf::Descriptor &msg) {
  std::vector<std::string> field_types;
  for (int i = 0; i < msg.field_count(); ++i) {
    const google::protobuf::FieldDescriptor *field = msg.field(i);
    switch (field->cpp_type()) {
      case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
      case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
      case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
      case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
      case google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
        field_types.push_back(field->cpp_type_name());
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
        // cpp_type_name returns "string".
        field_types.push_back("std::string");
        break;
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
        break;
        field_types.push_back(DotsToColons(field->enum_type()->full_name()));
      }
      case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
        field_types.push_back(DotsToColons(field->message_type()->full_name()));
        break;
      }
      default: {
        CHECK(false) << "Unreachable code.";
        break;
      }
    }
  }
  return field_types;
}

std::vector<std::string> FieldNames(const google::protobuf::Descriptor &msg) {
  std::vector<std::string> field_names;
  for (int i = 0; i < msg.field_count(); ++i) {
    const google::protobuf::FieldDescriptor *field = msg.field(i);
    field_names.push_back(field->name());
  }
  return field_names;
}

}  // namespace

#if 0
template <class T, size_t N>
T *array_end(T (&array)[N]) {
  return array + N;
}
#endif

std::string GetPrologue(ProtoBufFile *file, const Parameters &params) {
  UNUSED(params);

  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;

    vars["filename"] = file->filename();
    vars["filename_identifier"] = FilenameIdentifier(file->filename());
    vars["filename_base"] = file->filename_without_ext();
    vars["message_header_ext"] = kCppGeneratorMessageHeaderExt;

    printer->Print(vars, "// Generated by the gRPC fluent plugin.\n");
    printer->Print(vars,
                   "// If you make any local change, they will be lost.\n");
    printer->Print(vars, "// source: $filename$\n");
    std::string leading_comments = file->GetLeadingComments("//");
    if (!leading_comments.empty()) {
      printer->Print(vars, "// Original file comments:\n");
      printer->Print(leading_comments.c_str());
    }
    printer->Print(vars, "#ifndef FLUENT_$filename_identifier$__INCLUDED\n");
    printer->Print(vars, "#define FLUENT_$filename_identifier$__INCLUDED\n");
    printer->Print(vars, "\n");
    printer->Print(vars, "#include \"$filename_base$$message_header_ext$\"\n");
    printer->Print(vars, file->additional_headers().c_str());
    printer->Print(vars, "\n");
  }
  return output;
}

std::string GetIncludes(ProtoBufFile *file, const Parameters &params) {
  UNUSED(params);

  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;

    printer->Print(vars, "#include <cstdint>\n");
    printer->Print(vars, "\n");
    printer->Print(vars, "#include <string>\n");
    printer->Print(vars, "#include <tuple>\n");
    printer->Print(vars, "#include <utility>\n");
    printer->Print(vars, "\n");
    printer->Print(vars, "#include \"fluent/infix.h\"\n");
    printer->Print(vars, "#include \"grpc++/grpc++.h\"\n");
    printer->Print(vars, "\n");
    printer->Print(vars, "#include \"examples/grpc/api.grpc.pb.h\"\n");
    printer->Print(vars, "#include \"examples/grpc/api.pb.h\"\n");
    printer->Print(vars, "#include \"ra/logical/all.h\"\n");
    printer->Print(vars, "\n");

    if (!file->package().empty()) {
      std::vector<std::string> parts = file->package_parts();
      for (auto part = parts.begin(); part != parts.end(); part++) {
        vars["part"] = *part;
        printer->Print(vars, "namespace $part$ {\n");
      }
      printer->Print(vars, "\n");
    }
  }
  return output;
}

void PrintClientMethod(const fluent_generator::Method &method,
                       const Parameters &params,
                       fluent_generator::Printer *printer) {
  UNUSED(params);

  const google::protobuf::Descriptor *in_type = method.input_type();
  const std::vector<std::string> in_field_types = FieldTypes(*in_type);
  const std::vector<std::string> in_field_names = FieldNames(*in_type);

  const google::protobuf::Descriptor *out_type = method.output_type();
  const std::vector<std::string> out_field_types = FieldTypes(*out_type);
  const std::vector<std::string> out_field_names = FieldNames(*out_type);

  std::map<std::string, std::string> vars;
  vars["request_type"] = DotsToColons(in_type->full_name());
  vars["reply_type"] = DotsToColons(out_type->full_name());
  vars["method_name"] = method.name();

  // Method signature.
  vars["output_type"] =
      fmt::format("std::tuple<{}>", fluent::Join(out_field_types));
  printer->Print(vars, "$output_type$ ");
  printer->Print(vars, "$method_name$(");
  for (std::size_t j = 0; j < in_field_types.size(); ++j) {
    vars["input_type"] = in_field_types[j];
    vars["input_name"] = in_field_names[j];
    printer->Print(vars, "const $input_type$& $input_name$");
    if (j != in_field_types.size() - 1) {
      printer->Print(vars, ", ");
    }
  }
  printer->Print(vars, ") {\n");
  printer->Indent();

  // Request.
  printer->Print(vars, "// Request.\n");
  printer->Print(vars, "$request_type$ request;\n");
  for (std::size_t j = 0; j < in_field_types.size(); ++j) {
    const google::protobuf::FieldDescriptor *field = in_type->field(j);
    vars["input_type"] = in_field_types[j];
    vars["input_name"] = in_field_names[j];
    if (field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
      printer->Print(vars,
                     "*request->mutable_$input_name$() = $input_name$;\n");
    } else {
      printer->Print(vars, "request->set_$input_name$($input_name$);\n");
    }
  }
  printer->Print(vars, "\n");

  // Reply.
  printer->Print(vars, "// Reply.\n");
  printer->Print(vars, "$reply_type$ reply;\n\n");

  // RPC call.
  printer->Print(vars, "// RPC call.\n");
  printer->Print(vars, "grpc::ClientContext context;\n");
  printer->Print(vars,
                 "grpc::Status status = stub_->$method_name$("
                 "&context, request, reply);\n");
  printer->Print(vars, "CHECK(status.ok());\n\n");

  // Return.
  printer->Print(vars, "return $output_type$(");
  for (std::size_t j = 0; j < out_field_types.size(); ++j) {
    vars["output_name"] = out_field_names[j];
    printer->Print(vars, "reply->$output_name$()");
    if (j != in_field_types.size() - 1) {
      printer->Print(vars, ", ");
    }
  }
  printer->Print(vars, ");\n");

  // Method end.
  printer->Outdent();
  printer->Print(vars, "}\n\n");
}

std::string GetClientClass(ProtoBufFile *file, const Parameters &params) {
  UNUSED(params);
  CHECK_EQ(file->service_count(), 1);
  std::unique_ptr<const fluent_generator::Service> service = file->service(0);

  std::string output;
  {
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;
    vars["Service"] = service->name();

    // Class.
    printer->Print(service->GetLeadingComments("//").c_str());
    printer->Print(vars, "class $Service$Client final {\n");
    printer->Print(vars, " public:\n");
    printer->Indent();

    // Constructor.
    printer->Print(vars,
                   "$Service$Client(std::shared_ptr<grpc::Channel> channel)");
    printer->Print(vars, " : stub_($Service$::NewStub(channel)) {}");
    printer->Print(vars, "\n\n");

    // Methods.
    for (int i = 0; i < service->method_count(); ++i) {
      std::unique_ptr<const fluent_generator::Method> method =
          service->method(i);
      PrintClientMethod(*method, params, printer.get());
    }

    // Private members.
    printer->Outdent();
    printer->Print(vars, " private:\n");
    printer->Print(vars, "  std::unique_ptr<$Service$::Stub> stub_;\n");
    printer->Print(vars, "};\n\n");
  }
  return output;
}

void PrintMethodCollections(const fluent_generator::Method &method,
                            const Parameters &params,
                            fluent_generator::Printer *printer) {
  UNUSED(params);

  const google::protobuf::Descriptor *in_type = method.input_type();
  const std::vector<std::string> in_field_types = FieldTypes(*in_type);
  const std::vector<std::string> in_field_names = FieldNames(*in_type);

  const google::protobuf::Descriptor *out_type = method.output_type();
  const std::vector<std::string> out_field_types = FieldTypes(*out_type);
  const std::vector<std::string> out_field_names = FieldNames(*out_type);

  std::map<std::string, std::string> vars;
  vars["request_types"] = fluent::Join(in_field_types);
  vars["reply_types"] = fluent::Join(out_field_types);
  vars["method_name"] = method.name();

  // Request channel.
  printer->Print(vars,
                 ".channel<"
                 "std::string, std::string, std::int64_t, $request_types$"
                 ">(\"$method_name$_request\", "
                 "{{\"dst_addr\", \"src_addr\", \"id\", ");
  for (std::size_t i = 0; i < in_field_names.size(); ++i) {
    vars["column_name"] = in_field_names[i];
    printer->Print(vars, "\"$column_name$\"");
    if (i != in_field_names.size() - 1) {
      printer->Print(vars, ", ");
    }
  }
  printer->Print(vars, "}})\n");

  // Reply channel.
  printer->Print(vars,
                 ".channel<std::string, std::int64_t, $reply_types$>"
                 "(\"$method_name$_reply\", "
                 "{{\"addr\", \"id\", ");
  for (std::size_t i = 0; i < out_field_names.size(); ++i) {
    vars["column_name"] = out_field_names[i];
    printer->Print(vars, "\"$column_name$\"");
    if (i != out_field_names.size() - 1) {
      printer->Print(vars, ", ");
    }
  }
  printer->Print(vars, "}})\n");
}

std::string GetApiFunction(ProtoBufFile *file, const Parameters &params) {
  UNUSED(params);
  CHECK_EQ(file->service_count(), 1);
  std::unique_ptr<const fluent_generator::Service> service = file->service(0);

  std::string output;
  {
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;
    vars["Service"] = service->name();

    // Method signature.
    printer->Print(vars, "template <typename FluentBuilder>\n");
    printer->Print(vars, "auto Add$Service$Api(FluentBuilder f) {\n");
    printer->Indent();

    // Method body.
    printer->Print(vars, "return std::move(f)\n");
    printer->Indent();
    for (int i = 0; i < service->method_count(); ++i) {
      std::unique_ptr<const fluent_generator::Method> method =
          service->method(i);
      PrintMethodCollections(*method, params, printer.get());
    }
    printer->Outdent();
    printer->Print(vars, ";\n");

    // Method end.
    printer->Outdent();
    printer->Print(vars, "}\n\n");
  }
  return output;
}

std::string GetFluentFunction(ProtoBufFile *file, const Parameters &params) {
  UNUSED(file);
  UNUSED(params);
  return "";
}

std::string GetEpilogue(ProtoBufFile *file, const Parameters &params) {
  UNUSED(file);
  UNUSED(params);
  return "";
}

#if 0
void PrintIncludes(fluent_generator::Printer *printer,
                   const std::vector<std::string> &headers,
                   const Parameters &params) {
  std::map<std::string, std::string> vars;

  vars["l"] = params.use_system_headers ? '<' : '"';
  vars["r"] = params.use_system_headers ? '>' : '"';

  auto &s = params.grpc_search_path;
  if (!s.empty()) {
    vars["l"] += s;
    if (s[s.size() - 1] != '/') {
      vars["l"] += '/';
    }
  }

  for (auto i = headers.begin(); i != headers.end(); i++) {
    vars["h"] = *i;
    printer->Print(vars, "#include $l$$h$$r$\n");
  }
}
#endif
std::string GetHeaderPrologue(ProtoBufFile *file,
                              const Parameters & /*params*/) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the
    // string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;

    vars["filename"] = file->filename();
    vars["filename_identifier"] = FilenameIdentifier(file->filename());
    vars["filename_base"] = file->filename_without_ext();
    vars["message_header_ext"] = kCppGeneratorMessageHeaderExt;

    printer->Print(vars, "// Generated by the gRPC C++ plugin.\n");
    printer->Print(vars,
                   "// If you make any local change, they will be lost.\n");
    printer->Print(vars, "// source: $filename$\n");
    std::string leading_comments = file->GetLeadingComments("//");
    if (!leading_comments.empty()) {
      printer->Print(vars, "// Original file comments:\n");
      printer->Print(leading_comments.c_str());
    }
    printer->Print(vars, "#ifndef GRPC_$filename_identifier$__INCLUDED\n");
    printer->Print(vars, "#define GRPC_$filename_identifier$__INCLUDED\n");
    printer->Print(vars, "\n");
    printer->Print(vars, "#include \"$filename_base$$message_header_ext$\"\n");
    printer->Print(vars, file->additional_headers().c_str());
    printer->Print(vars, "\n");
  }
  return output;
}
#if 0

std::string GetHeaderIncludes(ProtoBufFile *file,
                              const Parameters &params) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;

    static const char *headers_strs[] = {
        "grpc++/impl/codegen/async_stream.h",
        "grpc++/impl/codegen/async_unary_call.h",
        "grpc++/impl/codegen/method_handler_impl.h",
        "grpc++/impl/codegen/proto_utils.h",
        "grpc++/impl/codegen/rpc_method.h",
        "grpc++/impl/codegen/service_type.h",
        "grpc++/impl/codegen/status.h",
        "grpc++/impl/codegen/stub_options.h",
        "grpc++/impl/codegen/sync_stream.h"};
    std::vector<std::string> headers(headers_strs, array_end(headers_strs));
    PrintIncludes(printer.get(), headers, params);
    printer->Print(vars, "\n");
    printer->Print(vars, "namespace grpc {\n");
    printer->Print(vars, "class CompletionQueue;\n");
    printer->Print(vars, "class Channel;\n");
    printer->Print(vars, "class RpcService;\n");
    printer->Print(vars, "class ServerCompletionQueue;\n");
    printer->Print(vars, "class ServerContext;\n");
    printer->Print(vars, "}  // namespace grpc\n\n");

    if (!file->package().empty()) {
      std::vector<std::string> parts = file->package_parts();

      for (auto part = parts.begin(); part != parts.end(); part++) {
        vars["part"] = *part;
        printer->Print(vars, "namespace $part$ {\n");
      }
      printer->Print(vars, "\n");
    }
  }
  return output;
}

void PrintHeaderClientMethodInterfaces(fluent_generator::Printer *printer,
                                       const fluent_generator::Method *method,
                                       std::map<std::string, std::string> *vars,
                                       bool is_public) {
  (*vars)["Method"] = method->name();
  (*vars)["Request"] = method->input_type_name();
  (*vars)["Response"] = method->output_type_name();

  if (is_public) {
    if (method->NoStreaming()) {
      printer->Print(
          *vars,
          "virtual ::grpc::Status $Method$(::grpc::ClientContext* context, "
          "const $Request$& request, $Response$* response) = 0;\n");
      printer->Print(*vars,
                     "std::unique_ptr< "
                     "::grpc::ClientAsyncResponseReaderInterface< $Response$>> "
                     "Async$Method$(::grpc::ClientContext* context, "
                     "const $Request$& request, "
                     "::grpc::CompletionQueue* cq) {\n");
      printer->Indent();
      printer->Print(*vars,
                     "return std::unique_ptr< "
                     "::grpc::ClientAsyncResponseReaderInterface< $Response$>>("
                     "Async$Method$Raw(context, request, cq));\n");
      printer->Outdent();
      printer->Print("}\n");
    } else if (ClientOnlyStreaming(method)) {
      printer->Print(
          *vars,
          "std::unique_ptr< ::grpc::ClientWriterInterface< $Request$>>"
          " $Method$("
          "::grpc::ClientContext* context, $Response$* response) {\n");
      printer->Indent();
      printer->Print(
          *vars,
          "return std::unique_ptr< ::grpc::ClientWriterInterface< $Request$>>"
          "($Method$Raw(context, response));\n");
      printer->Outdent();
      printer->Print("}\n");
      printer->Print(
          *vars,
          "std::unique_ptr< ::grpc::ClientAsyncWriterInterface< $Request$>>"
          " Async$Method$(::grpc::ClientContext* context, $Response$* "
          "response, "
          "::grpc::CompletionQueue* cq, void* tag) {\n");
      printer->Indent();
      printer->Print(*vars,
                     "return std::unique_ptr< "
                     "::grpc::ClientAsyncWriterInterface< $Request$>>("
                     "Async$Method$Raw(context, response, cq, tag));\n");
      printer->Outdent();
      printer->Print("}\n");
    } else if (ServerOnlyStreaming(method)) {
      printer->Print(
          *vars,
          "std::unique_ptr< ::grpc::ClientReaderInterface< $Response$>>"
          " $Method$(::grpc::ClientContext* context, const $Request$& request)"
          " {\n");
      printer->Indent();
      printer->Print(
          *vars,
          "return std::unique_ptr< ::grpc::ClientReaderInterface< $Response$>>"
          "($Method$Raw(context, request));\n");
      printer->Outdent();
      printer->Print("}\n");
      printer->Print(
          *vars,
          "std::unique_ptr< ::grpc::ClientAsyncReaderInterface< $Response$>> "
          "Async$Method$("
          "::grpc::ClientContext* context, const $Request$& request, "
          "::grpc::CompletionQueue* cq, void* tag) {\n");
      printer->Indent();
      printer->Print(*vars,
                     "return std::unique_ptr< "
                     "::grpc::ClientAsyncReaderInterface< $Response$>>("
                     "Async$Method$Raw(context, request, cq, tag));\n");
      printer->Outdent();
      printer->Print("}\n");
    } else if (method->BidiStreaming()) {
      printer->Print(*vars,
                     "std::unique_ptr< ::grpc::ClientReaderWriterInterface< "
                     "$Request$, $Response$>> "
                     "$Method$(::grpc::ClientContext* context) {\n");
      printer->Indent();
      printer->Print(
          *vars,
          "return std::unique_ptr< "
          "::grpc::ClientReaderWriterInterface< $Request$, $Response$>>("
          "$Method$Raw(context));\n");
      printer->Outdent();
      printer->Print("}\n");
      printer->Print(
          *vars,
          "std::unique_ptr< "
          "::grpc::ClientAsyncReaderWriterInterface< $Request$, $Response$>> "
          "Async$Method$(::grpc::ClientContext* context, "
          "::grpc::CompletionQueue* cq, void* tag) {\n");
      printer->Indent();
      printer->Print(
          *vars,
          "return std::unique_ptr< "
          "::grpc::ClientAsyncReaderWriterInterface< $Request$, $Response$>>("
          "Async$Method$Raw(context, cq, tag));\n");
      printer->Outdent();
      printer->Print("}\n");
    }
  } else {
    if (method->NoStreaming()) {
      printer->Print(
          *vars,
          "virtual ::grpc::ClientAsyncResponseReaderInterface< $Response$>* "
          "Async$Method$Raw(::grpc::ClientContext* context, "
          "const $Request$& request, "
          "::grpc::CompletionQueue* cq) = 0;\n");
    } else if (ClientOnlyStreaming(method)) {
      printer->Print(
          *vars,
          "virtual ::grpc::ClientWriterInterface< $Request$>*"
          " $Method$Raw("
          "::grpc::ClientContext* context, $Response$* response) = 0;\n");
      printer->Print(*vars,
                     "virtual ::grpc::ClientAsyncWriterInterface< $Request$>*"
                     " Async$Method$Raw(::grpc::ClientContext* context, "
                     "$Response$* response, "
                     "::grpc::CompletionQueue* cq, void* tag) = 0;\n");
    } else if (ServerOnlyStreaming(method)) {
      printer->Print(
          *vars,
          "virtual ::grpc::ClientReaderInterface< $Response$>* $Method$Raw("
          "::grpc::ClientContext* context, const $Request$& request) = 0;\n");
      printer->Print(
          *vars,
          "virtual ::grpc::ClientAsyncReaderInterface< $Response$>* "
          "Async$Method$Raw("
          "::grpc::ClientContext* context, const $Request$& request, "
          "::grpc::CompletionQueue* cq, void* tag) = 0;\n");
    } else if (method->BidiStreaming()) {
      printer->Print(*vars,
                     "virtual ::grpc::ClientReaderWriterInterface< $Request$, "
                     "$Response$>* "
                     "$Method$Raw(::grpc::ClientContext* context) = 0;\n");
      printer->Print(*vars,
                     "virtual ::grpc::ClientAsyncReaderWriterInterface< "
                     "$Request$, $Response$>* "
                     "Async$Method$Raw(::grpc::ClientContext* context, "
                     "::grpc::CompletionQueue* cq, void* tag) = 0;\n");
    }
  }
}

void PrintHeaderClientMethod(fluent_generator::Printer *printer,
                             const fluent_generator::Method *method,
                             std::map<std::string, std::string> *vars,
                             bool is_public) {
  (*vars)["Method"] = method->name();
  (*vars)["Request"] = method->input_type_name();
  (*vars)["Response"] = method->output_type_name();
  if (is_public) {
    if (method->NoStreaming()) {
      printer->Print(
          *vars,
          "::grpc::Status $Method$(::grpc::ClientContext* context, "
          "const $Request$& request, $Response$* response) override;\n");
      printer->Print(
          *vars,
          "std::unique_ptr< ::grpc::ClientAsyncResponseReader< $Response$>> "
          "Async$Method$(::grpc::ClientContext* context, "
          "const $Request$& request, "
          "::grpc::CompletionQueue* cq) {\n");
      printer->Indent();
      printer->Print(*vars,
                     "return std::unique_ptr< "
                     "::grpc::ClientAsyncResponseReader< $Response$>>("
                     "Async$Method$Raw(context, request, cq));\n");
      printer->Outdent();
      printer->Print("}\n");
    } else if (ClientOnlyStreaming(method)) {
      printer->Print(
          *vars,
          "std::unique_ptr< ::grpc::ClientWriter< $Request$>>"
          " $Method$("
          "::grpc::ClientContext* context, $Response$* response) {\n");
      printer->Indent();
      printer->Print(*vars,
                     "return std::unique_ptr< ::grpc::ClientWriter< $Request$>>"
                     "($Method$Raw(context, response));\n");
      printer->Outdent();
      printer->Print("}\n");
      printer->Print(*vars,
                     "std::unique_ptr< ::grpc::ClientAsyncWriter< $Request$>>"
                     " Async$Method$(::grpc::ClientContext* context, "
                     "$Response$* response, "
                     "::grpc::CompletionQueue* cq, void* tag) {\n");
      printer->Indent();
      printer->Print(
          *vars,
          "return std::unique_ptr< ::grpc::ClientAsyncWriter< $Request$>>("
          "Async$Method$Raw(context, response, cq, tag));\n");
      printer->Outdent();
      printer->Print("}\n");
    } else if (ServerOnlyStreaming(method)) {
      printer->Print(
          *vars,
          "std::unique_ptr< ::grpc::ClientReader< $Response$>>"
          " $Method$(::grpc::ClientContext* context, const $Request$& request)"
          " {\n");
      printer->Indent();
      printer->Print(
          *vars,
          "return std::unique_ptr< ::grpc::ClientReader< $Response$>>"
          "($Method$Raw(context, request));\n");
      printer->Outdent();
      printer->Print("}\n");
      printer->Print(
          *vars,
          "std::unique_ptr< ::grpc::ClientAsyncReader< $Response$>> "
          "Async$Method$("
          "::grpc::ClientContext* context, const $Request$& request, "
          "::grpc::CompletionQueue* cq, void* tag) {\n");
      printer->Indent();
      printer->Print(
          *vars,
          "return std::unique_ptr< ::grpc::ClientAsyncReader< $Response$>>("
          "Async$Method$Raw(context, request, cq, tag));\n");
      printer->Outdent();
      printer->Print("}\n");
    } else if (method->BidiStreaming()) {
      printer->Print(
          *vars,
          "std::unique_ptr< ::grpc::ClientReaderWriter< $Request$, $Response$>>"
          " $Method$(::grpc::ClientContext* context) {\n");
      printer->Indent();
      printer->Print(*vars,
                     "return std::unique_ptr< "
                     "::grpc::ClientReaderWriter< $Request$, $Response$>>("
                     "$Method$Raw(context));\n");
      printer->Outdent();
      printer->Print("}\n");
      printer->Print(*vars,
                     "std::unique_ptr<  ::grpc::ClientAsyncReaderWriter< "
                     "$Request$, $Response$>> "
                     "Async$Method$(::grpc::ClientContext* context, "
                     "::grpc::CompletionQueue* cq, void* tag) {\n");
      printer->Indent();
      printer->Print(*vars,
                     "return std::unique_ptr< "
                     "::grpc::ClientAsyncReaderWriter< $Request$, $Response$>>("
                     "Async$Method$Raw(context, cq, tag));\n");
      printer->Outdent();
      printer->Print("}\n");
    }
  } else {
    if (method->NoStreaming()) {
      printer->Print(*vars,
                     "::grpc::ClientAsyncResponseReader< $Response$>* "
                     "Async$Method$Raw(::grpc::ClientContext* context, "
                     "const $Request$& request, "
                     "::grpc::CompletionQueue* cq) override;\n");
    } else if (ClientOnlyStreaming(method)) {
      printer->Print(*vars,
                     "::grpc::ClientWriter< $Request$>* $Method$Raw("
                     "::grpc::ClientContext* context, $Response$* response) "
                     "override;\n");
      printer->Print(*vars,
                     "::grpc::ClientAsyncWriter< $Request$>* Async$Method$Raw("
                     "::grpc::ClientContext* context, $Response$* response, "
                     "::grpc::CompletionQueue* cq, void* tag) override;\n");
    } else if (ServerOnlyStreaming(method)) {
      printer->Print(*vars,
                     "::grpc::ClientReader< $Response$>* $Method$Raw("
                     "::grpc::ClientContext* context, const $Request$& request)"
                     " override;\n");
      printer->Print(
          *vars,
          "::grpc::ClientAsyncReader< $Response$>* Async$Method$Raw("
          "::grpc::ClientContext* context, const $Request$& request, "
          "::grpc::CompletionQueue* cq, void* tag) override;\n");
    } else if (method->BidiStreaming()) {
      printer->Print(*vars,
                     "::grpc::ClientReaderWriter< $Request$, $Response$>* "
                     "$Method$Raw(::grpc::ClientContext* context) override;\n");
      printer->Print(*vars,
                     "::grpc::ClientAsyncReaderWriter< $Request$, $Response$>* "
                     "Async$Method$Raw(::grpc::ClientContext* context, "
                     "::grpc::CompletionQueue* cq, void* tag) override;\n");
    }
  }
}

void PrintHeaderClientMethodData(fluent_generator::Printer *printer,
                                 const fluent_generator::Method *method,
                                 std::map<std::string, std::string> *vars) {
  (*vars)["Method"] = method->name();
  printer->Print(*vars, "const ::grpc::RpcMethod rpcmethod_$Method$_;\n");
}

void PrintHeaderServerMethodSync(fluent_generator::Printer *printer,
                                 const fluent_generator::Method *method,
                                 std::map<std::string, std::string> *vars) {
  (*vars)["Method"] = method->name();
  (*vars)["Request"] = method->input_type_name();
  (*vars)["Response"] = method->output_type_name();
  printer->Print(method->GetLeadingComments("//").c_str());
  if (method->NoStreaming()) {
    printer->Print(*vars,
                   "virtual ::grpc::Status $Method$("
                   "::grpc::ServerContext* context, const $Request$* request, "
                   "$Response$* response);\n");
  } else if (ClientOnlyStreaming(method)) {
    printer->Print(*vars,
                   "virtual ::grpc::Status $Method$("
                   "::grpc::ServerContext* context, "
                   "::grpc::ServerReader< $Request$>* reader, "
                   "$Response$* response);\n");
  } else if (ServerOnlyStreaming(method)) {
    printer->Print(*vars,
                   "virtual ::grpc::Status $Method$("
                   "::grpc::ServerContext* context, const $Request$* request, "
                   "::grpc::ServerWriter< $Response$>* writer);\n");
  } else if (method->BidiStreaming()) {
    printer->Print(
        *vars,
        "virtual ::grpc::Status $Method$("
        "::grpc::ServerContext* context, "
        "::grpc::ServerReaderWriter< $Response$, $Request$>* stream);"
        "\n");
  }
  printer->Print(method->GetTrailingComments("//").c_str());
}

void PrintHeaderServerMethodAsync(fluent_generator::Printer *printer,
                                  const fluent_generator::Method *method,
                                  std::map<std::string, std::string> *vars) {
  (*vars)["Method"] = method->name();
  (*vars)["Request"] = method->input_type_name();
  (*vars)["Response"] = method->output_type_name();
  printer->Print(*vars, "template <class BaseClass>\n");
  printer->Print(*vars,
                 "class WithAsyncMethod_$Method$ : public BaseClass {\n");
  printer->Print(
      " private:\n"
      "  void BaseClassMustBeDerivedFromService(const Service *service) {}\n");
  printer->Print(" public:\n");
  printer->Indent();
  printer->Print(*vars,
                 "WithAsyncMethod_$Method$() {\n"
                 "  ::grpc::Service::MarkMethodAsync($Idx$);\n"
                 "}\n");
  printer->Print(*vars,
                 "~WithAsyncMethod_$Method$() override {\n"
                 "  BaseClassMustBeDerivedFromService(this);\n"
                 "}\n");
  if (method->NoStreaming()) {
    printer->Print(
        *vars,
        "// disable synchronous version of this method\n"
        "::grpc::Status $Method$("
        "::grpc::ServerContext* context, const $Request$* request, "
        "$Response$* response) final override {\n"
        "  abort();\n"
        "  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, \"\");\n"
        "}\n");
    printer->Print(
        *vars,
        "void Request$Method$("
        "::grpc::ServerContext* context, $Request$* request, "
        "::grpc::ServerAsyncResponseWriter< $Response$>* response, "
        "::grpc::CompletionQueue* new_call_cq, "
        "::grpc::ServerCompletionQueue* notification_cq, void *tag) {\n");
    printer->Print(*vars,
                   "  ::grpc::Service::RequestAsyncUnary($Idx$, context, "
                   "request, response, new_call_cq, notification_cq, tag);\n");
    printer->Print("}\n");
  } else if (ClientOnlyStreaming(method)) {
    printer->Print(
        *vars,
        "// disable synchronous version of this method\n"
        "::grpc::Status $Method$("
        "::grpc::ServerContext* context, "
        "::grpc::ServerReader< $Request$>* reader, "
        "$Response$* response) final override {\n"
        "  abort();\n"
        "  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, \"\");\n"
        "}\n");
    printer->Print(
        *vars,
        "void Request$Method$("
        "::grpc::ServerContext* context, "
        "::grpc::ServerAsyncReader< $Response$, $Request$>* reader, "
        "::grpc::CompletionQueue* new_call_cq, "
        "::grpc::ServerCompletionQueue* notification_cq, void *tag) {\n");
    printer->Print(*vars,
                   "  ::grpc::Service::RequestAsyncClientStreaming($Idx$, "
                   "context, reader, new_call_cq, notification_cq, tag);\n");
    printer->Print("}\n");
  } else if (ServerOnlyStreaming(method)) {
    printer->Print(
        *vars,
        "// disable synchronous version of this method\n"
        "::grpc::Status $Method$("
        "::grpc::ServerContext* context, const $Request$* request, "
        "::grpc::ServerWriter< $Response$>* writer) final override "
        "{\n"
        "  abort();\n"
        "  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, \"\");\n"
        "}\n");
    printer->Print(
        *vars,
        "void Request$Method$("
        "::grpc::ServerContext* context, $Request$* request, "
        "::grpc::ServerAsyncWriter< $Response$>* writer, "
        "::grpc::CompletionQueue* new_call_cq, "
        "::grpc::ServerCompletionQueue* notification_cq, void *tag) {\n");
    printer->Print(
        *vars,
        "  ::grpc::Service::RequestAsyncServerStreaming($Idx$, "
        "context, request, writer, new_call_cq, notification_cq, tag);\n");
    printer->Print("}\n");
  } else if (method->BidiStreaming()) {
    printer->Print(
        *vars,
        "// disable synchronous version of this method\n"
        "::grpc::Status $Method$("
        "::grpc::ServerContext* context, "
        "::grpc::ServerReaderWriter< $Response$, $Request$>* stream) "
        "final override {\n"
        "  abort();\n"
        "  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, \"\");\n"
        "}\n");
    printer->Print(
        *vars,
        "void Request$Method$("
        "::grpc::ServerContext* context, "
        "::grpc::ServerAsyncReaderWriter< $Response$, $Request$>* stream, "
        "::grpc::CompletionQueue* new_call_cq, "
        "::grpc::ServerCompletionQueue* notification_cq, void *tag) {\n");
    printer->Print(*vars,
                   "  ::grpc::Service::RequestAsyncBidiStreaming($Idx$, "
                   "context, stream, new_call_cq, notification_cq, tag);\n");
    printer->Print("}\n");
  }
  printer->Outdent();
  printer->Print(*vars, "};\n");
}

void PrintHeaderServerMethodStreamedUnary(
    fluent_generator::Printer *printer, const fluent_generator::Method *method,
    std::map<std::string, std::string> *vars) {
  (*vars)["Method"] = method->name();
  (*vars)["Request"] = method->input_type_name();
  (*vars)["Response"] = method->output_type_name();
  if (method->NoStreaming()) {
    printer->Print(*vars, "template <class BaseClass>\n");
    printer->Print(*vars,
                   "class WithStreamedUnaryMethod_$Method$ : "
                   "public BaseClass {\n");
    printer->Print(
        " private:\n"
        "  void BaseClassMustBeDerivedFromService(const Service *service) "
        "{}\n");
    printer->Print(" public:\n");
    printer->Indent();
    printer->Print(*vars,
                   "WithStreamedUnaryMethod_$Method$() {\n"
                   "  ::grpc::Service::MarkMethodStreamed($Idx$,\n"
                   "    new ::grpc::StreamedUnaryHandler< $Request$, "
                   "$Response$>(std::bind"
                   "(&WithStreamedUnaryMethod_$Method$<BaseClass>::"
                   "Streamed$Method$, this, std::placeholders::_1, "
                   "std::placeholders::_2)));\n"
                   "}\n");
    printer->Print(*vars,
                   "~WithStreamedUnaryMethod_$Method$() override {\n"
                   "  BaseClassMustBeDerivedFromService(this);\n"
                   "}\n");
    printer->Print(
        *vars,
        "// disable regular version of this method\n"
        "::grpc::Status $Method$("
        "::grpc::ServerContext* context, const $Request$* request, "
        "$Response$* response) final override {\n"
        "  abort();\n"
        "  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, \"\");\n"
        "}\n");
    printer->Print(*vars,
                   "// replace default version of method with streamed unary\n"
                   "virtual ::grpc::Status Streamed$Method$("
                   "::grpc::ServerContext* context, "
                   "::grpc::ServerUnaryStreamer< "
                   "$Request$,$Response$>* server_unary_streamer)"
                   " = 0;\n");
    printer->Outdent();
    printer->Print(*vars, "};\n");
  }
}

void PrintHeaderServerMethodSplitStreaming(
    fluent_generator::Printer *printer, const fluent_generator::Method *method,
    std::map<std::string, std::string> *vars) {
  (*vars)["Method"] = method->name();
  (*vars)["Request"] = method->input_type_name();
  (*vars)["Response"] = method->output_type_name();
  if (ServerOnlyStreaming(method)) {
    printer->Print(*vars, "template <class BaseClass>\n");
    printer->Print(*vars,
                   "class WithSplitStreamingMethod_$Method$ : "
                   "public BaseClass {\n");
    printer->Print(
        " private:\n"
        "  void BaseClassMustBeDerivedFromService(const Service *service) "
        "{}\n");
    printer->Print(" public:\n");
    printer->Indent();
    printer->Print(*vars,
                   "WithSplitStreamingMethod_$Method$() {\n"
                   "  ::grpc::Service::MarkMethodStreamed($Idx$,\n"
                   "    new ::grpc::SplitServerStreamingHandler< $Request$, "
                   "$Response$>(std::bind"
                   "(&WithSplitStreamingMethod_$Method$<BaseClass>::"
                   "Streamed$Method$, this, std::placeholders::_1, "
                   "std::placeholders::_2)));\n"
                   "}\n");
    printer->Print(*vars,
                   "~WithSplitStreamingMethod_$Method$() override {\n"
                   "  BaseClassMustBeDerivedFromService(this);\n"
                   "}\n");
    printer->Print(
        *vars,
        "// disable regular version of this method\n"
        "::grpc::Status $Method$("
        "::grpc::ServerContext* context, const $Request$* request, "
        "::grpc::ServerWriter< $Response$>* writer) final override "
        "{\n"
        "  abort();\n"
        "  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, \"\");\n"
        "}\n");
    printer->Print(*vars,
                   "// replace default version of method with split streamed\n"
                   "virtual ::grpc::Status Streamed$Method$("
                   "::grpc::ServerContext* context, "
                   "::grpc::ServerSplitStreamer< "
                   "$Request$,$Response$>* server_split_streamer)"
                   " = 0;\n");
    printer->Outdent();
    printer->Print(*vars, "};\n");
  }
}

void PrintHeaderServerMethodGeneric(fluent_generator::Printer *printer,
                                    const fluent_generator::Method *method,
                                    std::map<std::string, std::string> *vars) {
  (*vars)["Method"] = method->name();
  (*vars)["Request"] = method->input_type_name();
  (*vars)["Response"] = method->output_type_name();
  printer->Print(*vars, "template <class BaseClass>\n");
  printer->Print(*vars,
                 "class WithGenericMethod_$Method$ : public BaseClass {\n");
  printer->Print(
      " private:\n"
      "  void BaseClassMustBeDerivedFromService(const Service *service) {}\n");
  printer->Print(" public:\n");
  printer->Indent();
  printer->Print(*vars,
                 "WithGenericMethod_$Method$() {\n"
                 "  ::grpc::Service::MarkMethodGeneric($Idx$);\n"
                 "}\n");
  printer->Print(*vars,
                 "~WithGenericMethod_$Method$() override {\n"
                 "  BaseClassMustBeDerivedFromService(this);\n"
                 "}\n");
  if (method->NoStreaming()) {
    printer->Print(
        *vars,
        "// disable synchronous version of this method\n"
        "::grpc::Status $Method$("
        "::grpc::ServerContext* context, const $Request$* request, "
        "$Response$* response) final override {\n"
        "  abort();\n"
        "  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, \"\");\n"
        "}\n");
  } else if (ClientOnlyStreaming(method)) {
    printer->Print(
        *vars,
        "// disable synchronous version of this method\n"
        "::grpc::Status $Method$("
        "::grpc::ServerContext* context, "
        "::grpc::ServerReader< $Request$>* reader, "
        "$Response$* response) final override {\n"
        "  abort();\n"
        "  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, \"\");\n"
        "}\n");
  } else if (ServerOnlyStreaming(method)) {
    printer->Print(
        *vars,
        "// disable synchronous version of this method\n"
        "::grpc::Status $Method$("
        "::grpc::ServerContext* context, const $Request$* request, "
        "::grpc::ServerWriter< $Response$>* writer) final override "
        "{\n"
        "  abort();\n"
        "  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, \"\");\n"
        "}\n");
  } else if (method->BidiStreaming()) {
    printer->Print(
        *vars,
        "// disable synchronous version of this method\n"
        "::grpc::Status $Method$("
        "::grpc::ServerContext* context, "
        "::grpc::ServerReaderWriter< $Response$, $Request$>* stream) "
        "final override {\n"
        "  abort();\n"
        "  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, \"\");\n"
        "}\n");
  }
  printer->Outdent();
  printer->Print(*vars, "};\n");
}

void PrintHeaderService(fluent_generator::Printer *printer,
                        const fluent_generator::Service *service,
                        std::map<std::string, std::string> *vars) {
  (*vars)["Service"] = service->name();

  printer->Print(service->GetLeadingComments("//").c_str());
  printer->Print(*vars,
                 "class $Service$ final {\n"
                 " public:\n");
  printer->Indent();

  // Service metadata
  printer->Print(*vars,
                 "static constexpr char const* service_full_name() {\n"
                 "  return \"$Package$$Service$\";\n"
                 "}\n");

  // Client side
  printer->Print(
      "class StubInterface {\n"
      " public:\n");
  printer->Indent();
  printer->Print("virtual ~StubInterface() {}\n");
  for (int i = 0; i < service->method_count(); ++i) {
    printer->Print(service->method(i)->GetLeadingComments("//").c_str());
    PrintHeaderClientMethodInterfaces(printer, service->method(i).get(), vars,
                                      true);
    printer->Print(service->method(i)->GetTrailingComments("//").c_str());
  }
  printer->Outdent();
  printer->Print("private:\n");
  printer->Indent();
  for (int i = 0; i < service->method_count(); ++i) {
    PrintHeaderClientMethodInterfaces(printer, service->method(i).get(), vars,
                                      false);
  }
  printer->Outdent();
  printer->Print("};\n");
  printer->Print(
      "class Stub final : public StubInterface"
      " {\n public:\n");
  printer->Indent();
  printer->Print(
      "Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel);\n");
  for (int i = 0; i < service->method_count(); ++i) {
    PrintHeaderClientMethod(printer, service->method(i).get(), vars, true);
  }
  printer->Outdent();
  printer->Print("\n private:\n");
  printer->Indent();
  printer->Print("std::shared_ptr< ::grpc::ChannelInterface> channel_;\n");
  for (int i = 0; i < service->method_count(); ++i) {
    PrintHeaderClientMethod(printer, service->method(i).get(), vars, false);
  }
  for (int i = 0; i < service->method_count(); ++i) {
    PrintHeaderClientMethodData(printer, service->method(i).get(), vars);
  }
  printer->Outdent();
  printer->Print("};\n");
  printer->Print(
      "static std::unique_ptr<Stub> NewStub(const std::shared_ptr< "
      "::grpc::ChannelInterface>& channel, "
      "const ::grpc::StubOptions& options = ::grpc::StubOptions());\n");

  printer->Print("\n");

  // Server side - base
  printer->Print(
      "class Service : public ::grpc::Service {\n"
      " public:\n");
  printer->Indent();
  printer->Print("Service();\n");
  printer->Print("virtual ~Service();\n");
  for (int i = 0; i < service->method_count(); ++i) {
    PrintHeaderServerMethodSync(printer, service->method(i).get(), vars);
  }
  printer->Outdent();
  printer->Print("};\n");

  // Server side - Asynchronous
  for (int i = 0; i < service->method_count(); ++i) {
    (*vars)["Idx"] = as_string(i);
    PrintHeaderServerMethodAsync(printer, service->method(i).get(), vars);
  }

  printer->Print("typedef ");

  for (int i = 0; i < service->method_count(); ++i) {
    (*vars)["method_name"] = service->method(i).get()->name();
    printer->Print(*vars, "WithAsyncMethod_$method_name$<");
  }
  printer->Print("Service");
  for (int i = 0; i < service->method_count(); ++i) {
    printer->Print(" >");
  }
  printer->Print(" AsyncService;\n");

  // Server side - Generic
  for (int i = 0; i < service->method_count(); ++i) {
    (*vars)["Idx"] = as_string(i);
    PrintHeaderServerMethodGeneric(printer, service->method(i).get(), vars);
  }

  // Server side - Streamed Unary
  for (int i = 0; i < service->method_count(); ++i) {
    (*vars)["Idx"] = as_string(i);
    PrintHeaderServerMethodStreamedUnary(printer, service->method(i).get(),
                                         vars);
  }

  printer->Print("typedef ");
  for (int i = 0; i < service->method_count(); ++i) {
    (*vars)["method_name"] = service->method(i).get()->name();
    if (service->method(i)->NoStreaming()) {
      printer->Print(*vars, "WithStreamedUnaryMethod_$method_name$<");
    }
  }
  printer->Print("Service");
  for (int i = 0; i < service->method_count(); ++i) {
    if (service->method(i)->NoStreaming()) {
      printer->Print(" >");
    }
  }
  printer->Print(" StreamedUnaryService;\n");

  // Server side - controlled server-side streaming
  for (int i = 0; i < service->method_count(); ++i) {
    (*vars)["Idx"] = as_string(i);
    PrintHeaderServerMethodSplitStreaming(printer, service->method(i).get(),
                                          vars);
  }

  printer->Print("typedef ");
  for (int i = 0; i < service->method_count(); ++i) {
    (*vars)["method_name"] = service->method(i).get()->name();
    auto method = service->method(i);
    if (ServerOnlyStreaming(method.get())) {
      printer->Print(*vars, "WithSplitStreamingMethod_$method_name$<");
    }
  }
  printer->Print("Service");
  for (int i = 0; i < service->method_count(); ++i) {
    auto method = service->method(i);
    if (ServerOnlyStreaming(method.get())) {
      printer->Print(" >");
    }
  }
  printer->Print(" SplitStreamedService;\n");

  // Server side - typedef for controlled both unary and server-side streaming
  printer->Print("typedef ");
  for (int i = 0; i < service->method_count(); ++i) {
    (*vars)["method_name"] = service->method(i).get()->name();
    auto method = service->method(i);
    if (ServerOnlyStreaming(method.get())) {
      printer->Print(*vars, "WithSplitStreamingMethod_$method_name$<");
    }
    if (service->method(i)->NoStreaming()) {
      printer->Print(*vars, "WithStreamedUnaryMethod_$method_name$<");
    }
  }
  printer->Print("Service");
  for (int i = 0; i < service->method_count(); ++i) {
    auto method = service->method(i);
    if (service->method(i)->NoStreaming() ||
        ServerOnlyStreaming(method.get())) {
      printer->Print(" >");
    }
  }
  printer->Print(" StreamedService;\n");

  printer->Outdent();
  printer->Print("};\n");
  printer->Print(service->GetTrailingComments("//").c_str());
}

std::string GetHeaderServices(ProtoBufFile *file,
                              const Parameters &params) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;
    // Package string is empty or ends with a dot. It is used to fully qualify
    // method names.
    vars["Package"] = file->package();
    if (!file->package().empty()) {
      vars["Package"].append(".");
    }

    if (!params.services_namespace.empty()) {
      vars["services_namespace"] = params.services_namespace;
      printer->Print(vars, "\nnamespace $services_namespace$ {\n\n");
    }

    for (int i = 0; i < file->service_count(); ++i) {
      PrintHeaderService(printer.get(), file->service(i).get(), &vars);
      printer->Print("\n");
    }

    if (!params.services_namespace.empty()) {
      printer->Print(vars, "}  // namespace $services_namespace$\n\n");
    }
  }
  return output;
}

std::string GetHeaderEpilogue(ProtoBufFile *file,
                              const Parameters & /*params*/) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;

    vars["filename"] = file->filename();
    vars["filename_identifier"] = FilenameIdentifier(file->filename());

    if (!file->package().empty()) {
      std::vector<std::string> parts = file->package_parts();

      for (auto part = parts.rbegin(); part != parts.rend(); part++) {
        vars["part"] = *part;
        printer->Print(vars, "}  // namespace $part$\n");
      }
      printer->Print(vars, "\n");
    }

    printer->Print(vars, "\n");
    printer->Print(vars, "#endif  // GRPC_$filename_identifier$__INCLUDED\n");

    printer->Print(file->GetTrailingComments("//").c_str());
  }
  return output;
}

std::string GetSourcePrologue(ProtoBufFile *file,
                              const Parameters & /*params*/) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;

    vars["filename"] = file->filename();
    vars["filename_base"] = file->filename_without_ext();
    vars["message_header_ext"] = kCppGeneratorMessageHeaderExt;
    vars["service_header_ext"] = kCppGeneratorServiceHeaderExt;

    printer->Print(vars, "// Generated by the gRPC C++ plugin.\n");
    printer->Print(vars,
                   "// If you make any local change, they will be lost.\n");
    printer->Print(vars, "// source: $filename$\n\n");

    printer->Print(vars, "#include \"$filename_base$$message_header_ext$\"\n");
    printer->Print(vars, "#include \"$filename_base$$service_header_ext$\"\n");
    printer->Print(vars, "\n");
  }
  return output;
}

std::string GetSourceIncludes(ProtoBufFile *file,
                              const Parameters &params) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;

    static const char *headers_strs[] = {
        "grpc++/impl/codegen/async_stream.h",
        "grpc++/impl/codegen/async_unary_call.h",
        "grpc++/impl/codegen/channel_interface.h",
        "grpc++/impl/codegen/client_unary_call.h",
        "grpc++/impl/codegen/method_handler_impl.h",
        "grpc++/impl/codegen/rpc_service_method.h",
        "grpc++/impl/codegen/service_type.h",
        "grpc++/impl/codegen/sync_stream.h"};
    std::vector<std::string> headers(headers_strs, array_end(headers_strs));
    PrintIncludes(printer.get(), headers, params);

    if (!file->package().empty()) {
      std::vector<std::string> parts = file->package_parts();

      for (auto part = parts.begin(); part != parts.end(); part++) {
        vars["part"] = *part;
        printer->Print(vars, "namespace $part$ {\n");
      }
    }

    printer->Print(vars, "\n");
  }
  return output;
}

void PrintSourceClientMethod(fluent_generator::Printer *printer,
                             const fluent_generator::Method *method,
                             std::map<std::string, std::string> *vars) {
  (*vars)["Method"] = method->name();
  (*vars)["Request"] = method->input_type_name();
  (*vars)["Response"] = method->output_type_name();
  if (method->NoStreaming()) {
    printer->Print(*vars,
                   "::grpc::Status $ns$$Service$::Stub::$Method$("
                   "::grpc::ClientContext* context, "
                   "const $Request$& request, $Response$* response) {\n");
    printer->Print(*vars,
                   "  return ::grpc::BlockingUnaryCall(channel_.get(), "
                   "rpcmethod_$Method$_, "
                   "context, request, response);\n"
                   "}\n\n");
    printer->Print(
        *vars,
        "::grpc::ClientAsyncResponseReader< $Response$>* "
        "$ns$$Service$::Stub::Async$Method$Raw(::grpc::ClientContext* context, "
        "const $Request$& request, "
        "::grpc::CompletionQueue* cq) {\n");
    printer->Print(*vars,
                   "  return "
                   "::grpc::ClientAsyncResponseReader< $Response$>::Create("
                   "channel_.get(), cq, "
                   "rpcmethod_$Method$_, "
                   "context, request);\n"
                   "}\n\n");
  } else if (ClientOnlyStreaming(method)) {
    printer->Print(*vars,
                   "::grpc::ClientWriter< $Request$>* "
                   "$ns$$Service$::Stub::$Method$Raw("
                   "::grpc::ClientContext* context, $Response$* response) {\n");
    printer->Print(*vars,
                   "  return new ::grpc::ClientWriter< $Request$>("
                   "channel_.get(), "
                   "rpcmethod_$Method$_, "
                   "context, response);\n"
                   "}\n\n");
    printer->Print(*vars,
                   "::grpc::ClientAsyncWriter< $Request$>* "
                   "$ns$$Service$::Stub::Async$Method$Raw("
                   "::grpc::ClientContext* context, $Response$* response, "
                   "::grpc::CompletionQueue* cq, void* tag) {\n");
    printer->Print(*vars,
                   "  return ::grpc::ClientAsyncWriter< $Request$>::Create("
                   "channel_.get(), cq, "
                   "rpcmethod_$Method$_, "
                   "context, response, tag);\n"
                   "}\n\n");
  } else if (ServerOnlyStreaming(method)) {
    printer->Print(
        *vars,
        "::grpc::ClientReader< $Response$>* "
        "$ns$$Service$::Stub::$Method$Raw("
        "::grpc::ClientContext* context, const $Request$& request) {\n");
    printer->Print(*vars,
                   "  return new ::grpc::ClientReader< $Response$>("
                   "channel_.get(), "
                   "rpcmethod_$Method$_, "
                   "context, request);\n"
                   "}\n\n");
    printer->Print(*vars,
                   "::grpc::ClientAsyncReader< $Response$>* "
                   "$ns$$Service$::Stub::Async$Method$Raw("
                   "::grpc::ClientContext* context, const $Request$& request, "
                   "::grpc::CompletionQueue* cq, void* tag) {\n");
    printer->Print(*vars,
                   "  return ::grpc::ClientAsyncReader< $Response$>::Create("
                   "channel_.get(), cq, "
                   "rpcmethod_$Method$_, "
                   "context, request, tag);\n"
                   "}\n\n");
  } else if (method->BidiStreaming()) {
    printer->Print(
        *vars,
        "::grpc::ClientReaderWriter< $Request$, $Response$>* "
        "$ns$$Service$::Stub::$Method$Raw(::grpc::ClientContext* context) {\n");
    printer->Print(*vars,
                   "  return new ::grpc::ClientReaderWriter< "
                   "$Request$, $Response$>("
                   "channel_.get(), "
                   "rpcmethod_$Method$_, "
                   "context);\n"
                   "}\n\n");
    printer->Print(
        *vars,
        "::grpc::ClientAsyncReaderWriter< $Request$, $Response$>* "
        "$ns$$Service$::Stub::Async$Method$Raw(::grpc::ClientContext* context, "
        "::grpc::CompletionQueue* cq, void* tag) {\n");
    printer->Print(
        *vars,
        "  return "
        "::grpc::ClientAsyncReaderWriter< $Request$, $Response$>::Create("
        "channel_.get(), cq, "
        "rpcmethod_$Method$_, "
        "context, tag);\n"
        "}\n\n");
  }
}

void PrintSourceServerMethod(fluent_generator::Printer *printer,
                             const fluent_generator::Method *method,
                             std::map<std::string, std::string> *vars) {
  (*vars)["Method"] = method->name();
  (*vars)["Request"] = method->input_type_name();
  (*vars)["Response"] = method->output_type_name();
  if (method->NoStreaming()) {
    printer->Print(*vars,
                   "::grpc::Status $ns$$Service$::Service::$Method$("
                   "::grpc::ServerContext* context, "
                   "const $Request$* request, $Response$* response) {\n");
    printer->Print("  (void) context;\n");
    printer->Print("  (void) request;\n");
    printer->Print("  (void) response;\n");
    printer->Print(
        "  return ::grpc::Status("
        "::grpc::StatusCode::UNIMPLEMENTED, \"\");\n");
    printer->Print("}\n\n");
  } else if (ClientOnlyStreaming(method)) {
    printer->Print(*vars,
                   "::grpc::Status $ns$$Service$::Service::$Method$("
                   "::grpc::ServerContext* context, "
                   "::grpc::ServerReader< $Request$>* reader, "
                   "$Response$* response) {\n");
    printer->Print("  (void) context;\n");
    printer->Print("  (void) reader;\n");
    printer->Print("  (void) response;\n");
    printer->Print(
        "  return ::grpc::Status("
        "::grpc::StatusCode::UNIMPLEMENTED, \"\");\n");
    printer->Print("}\n\n");
  } else if (ServerOnlyStreaming(method)) {
    printer->Print(*vars,
                   "::grpc::Status $ns$$Service$::Service::$Method$("
                   "::grpc::ServerContext* context, "
                   "const $Request$* request, "
                   "::grpc::ServerWriter< $Response$>* writer) {\n");
    printer->Print("  (void) context;\n");
    printer->Print("  (void) request;\n");
    printer->Print("  (void) writer;\n");
    printer->Print(
        "  return ::grpc::Status("
        "::grpc::StatusCode::UNIMPLEMENTED, \"\");\n");
    printer->Print("}\n\n");
  } else if (method->BidiStreaming()) {
    printer->Print(*vars,
                   "::grpc::Status $ns$$Service$::Service::$Method$("
                   "::grpc::ServerContext* context, "
                   "::grpc::ServerReaderWriter< $Response$, $Request$>* "
                   "stream) {\n");
    printer->Print("  (void) context;\n");
    printer->Print("  (void) stream;\n");
    printer->Print(
        "  return ::grpc::Status("
        "::grpc::StatusCode::UNIMPLEMENTED, \"\");\n");
    printer->Print("}\n\n");
  }
}

void PrintSourceService(fluent_generator::Printer *printer,
                        const fluent_generator::Service *service,
                        std::map<std::string, std::string> *vars) {
  (*vars)["Service"] = service->name();

  if (service->method_count() > 0) {
    printer->Print(*vars,
                   "static const char* $prefix$$Service$_method_names[] = {\n");
    for (int i = 0; i < service->method_count(); ++i) {
      (*vars)["Method"] = service->method(i).get()->name();
      printer->Print(*vars, "  \"/$Package$$Service$/$Method$\",\n");
    }
    printer->Print(*vars, "};\n\n");
  }

  printer->Print(*vars,
                 "std::unique_ptr< $ns$$Service$::Stub> $ns$$Service$::NewStub("
                 "const std::shared_ptr< ::grpc::ChannelInterface>& channel, "
                 "const ::grpc::StubOptions& options) {\n"
                 "  std::unique_ptr< $ns$$Service$::Stub> stub(new "
                 "$ns$$Service$::Stub(channel));\n"
                 "  return stub;\n"
                 "}\n\n");
  printer->Print(*vars,
                 "$ns$$Service$::Stub::Stub(const std::shared_ptr< "
                 "::grpc::ChannelInterface>& channel)\n");
  printer->Indent();
  printer->Print(": channel_(channel)");
  for (int i = 0; i < service->method_count(); ++i) {
    auto method = service->method(i);
    (*vars)["Method"] = method->name();
    (*vars)["Idx"] = as_string(i);
    if (method->NoStreaming()) {
      (*vars)["StreamingType"] = "NORMAL_RPC";
      // NOTE: There is no reason to consider streamed-unary as a separate
      // category here since this part is setting up the client-side stub
      // and this appears as a NORMAL_RPC from the client-side.
    } else if (ClientOnlyStreaming(method.get())) {
      (*vars)["StreamingType"] = "CLIENT_STREAMING";
    } else if (ServerOnlyStreaming(method.get())) {
      (*vars)["StreamingType"] = "SERVER_STREAMING";
    } else {
      (*vars)["StreamingType"] = "BIDI_STREAMING";
    }
    printer->Print(*vars,
                   ", rpcmethod_$Method$_("
                   "$prefix$$Service$_method_names[$Idx$], "
                   "::grpc::RpcMethod::$StreamingType$, "
                   "channel"
                   ")\n");
  }
  printer->Print("{}\n\n");
  printer->Outdent();

  for (int i = 0; i < service->method_count(); ++i) {
    (*vars)["Idx"] = as_string(i);
    PrintSourceClientMethod(printer, service->method(i).get(), vars);
  }

  printer->Print(*vars, "$ns$$Service$::Service::Service() {\n");
  printer->Indent();
  for (int i = 0; i < service->method_count(); ++i) {
    auto method = service->method(i);
    (*vars)["Idx"] = as_string(i);
    (*vars)["Method"] = method->name();
    (*vars)["Request"] = method->input_type_name();
    (*vars)["Response"] = method->output_type_name();
    if (method->NoStreaming()) {
      printer->Print(
          *vars,
          "AddMethod(new ::grpc::RpcServiceMethod(\n"
          "    $prefix$$Service$_method_names[$Idx$],\n"
          "    ::grpc::RpcMethod::NORMAL_RPC,\n"
          "    new ::grpc::RpcMethodHandler< $ns$$Service$::Service, "
          "$Request$, "
          "$Response$>(\n"
          "        std::mem_fn(&$ns$$Service$::Service::$Method$), this)));\n");
    } else if (ClientOnlyStreaming(method.get())) {
      printer->Print(
          *vars,
          "AddMethod(new ::grpc::RpcServiceMethod(\n"
          "    $prefix$$Service$_method_names[$Idx$],\n"
          "    ::grpc::RpcMethod::CLIENT_STREAMING,\n"
          "    new ::grpc::ClientStreamingHandler< "
          "$ns$$Service$::Service, $Request$, $Response$>(\n"
          "        std::mem_fn(&$ns$$Service$::Service::$Method$), this)));\n");
    } else if (ServerOnlyStreaming(method.get())) {
      printer->Print(
          *vars,
          "AddMethod(new ::grpc::RpcServiceMethod(\n"
          "    $prefix$$Service$_method_names[$Idx$],\n"
          "    ::grpc::RpcMethod::SERVER_STREAMING,\n"
          "    new ::grpc::ServerStreamingHandler< "
          "$ns$$Service$::Service, $Request$, $Response$>(\n"
          "        std::mem_fn(&$ns$$Service$::Service::$Method$), this)));\n");
    } else if (method->BidiStreaming()) {
      printer->Print(
          *vars,
          "AddMethod(new ::grpc::RpcServiceMethod(\n"
          "    $prefix$$Service$_method_names[$Idx$],\n"
          "    ::grpc::RpcMethod::BIDI_STREAMING,\n"
          "    new ::grpc::BidiStreamingHandler< "
          "$ns$$Service$::Service, $Request$, $Response$>(\n"
          "        std::mem_fn(&$ns$$Service$::Service::$Method$), this)));\n");
    }
  }
  printer->Outdent();
  printer->Print(*vars, "}\n\n");
  printer->Print(*vars,
                 "$ns$$Service$::Service::~Service() {\n"
                 "}\n\n");
  for (int i = 0; i < service->method_count(); ++i) {
    (*vars)["Idx"] = as_string(i);
    PrintSourceServerMethod(printer, service->method(i).get(), vars);
  }
}

std::string GetSourceServices(ProtoBufFile *file,
                              const Parameters &params) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;
    // Package string is empty or ends with a dot. It is used to fully qualify
    // method names.
    vars["Package"] = file->package();
    if (!file->package().empty()) {
      vars["Package"].append(".");
    }
    if (!params.services_namespace.empty()) {
      vars["ns"] = params.services_namespace + "::";
      vars["prefix"] = params.services_namespace;
    } else {
      vars["ns"] = "";
      vars["prefix"] = "";
    }

    for (int i = 0; i < file->service_count(); ++i) {
      PrintSourceService(printer.get(), file->service(i).get(), &vars);
      printer->Print("\n");
    }
  }
  return output;
}

std::string GetSourceEpilogue(ProtoBufFile *file,
                              const Parameters & /*params*/) {
  std::string temp;

  if (!file->package().empty()) {
    std::vector<std::string> parts = file->package_parts();

    for (auto part = parts.begin(); part != parts.end(); part++) {
      temp.append("}  // namespace ");
      temp.append(*part);
      temp.append("\n");
    }
    temp.append("\n");
  }

  return temp;
}

// TODO(mmukhi): Make sure we need parameters or not.
std::string GetMockPrologue(ProtoBufFile *file,
                            const Parameters & /*params*/) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;

    vars["filename"] = file->filename();
    vars["filename_base"] = file->filename_without_ext();
    vars["message_header_ext"] = kCppGeneratorMessageHeaderExt;
    vars["service_header_ext"] = kCppGeneratorServiceHeaderExt;

    printer->Print(vars, "// Generated by the gRPC C++ plugin.\n");
    printer->Print(vars,
                   "// If you make any local change, they will be lost.\n");
    printer->Print(vars, "// source: $filename$\n\n");

    printer->Print(vars, "#include \"$filename_base$$message_header_ext$\"\n");
    printer->Print(vars, "#include \"$filename_base$$service_header_ext$\"\n");
    printer->Print(vars, file->additional_headers().c_str());
    printer->Print(vars, "\n");
  }
  return output;
}

// TODO(mmukhi): Add client-stream and completion-queue headers.
std::string GetMockIncludes(ProtoBufFile *file,
                            const Parameters &params) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;

    static const char *headers_strs[] = {
        "grpc++/impl/codegen/async_stream.h",
        "grpc++/impl/codegen/sync_stream.h", "gmock/gmock.h",
    };
    std::vector<std::string> headers(headers_strs, array_end(headers_strs));
    PrintIncludes(printer.get(), headers, params);

    if (!file->package().empty()) {
      std::vector<std::string> parts = file->package_parts();

      for (auto part = parts.begin(); part != parts.end(); part++) {
        vars["part"] = *part;
        printer->Print(vars, "namespace $part$ {\n");
      }
    }

    printer->Print(vars, "\n");
  }
  return output;
}

void PrintMockClientMethods(fluent_generator::Printer *printer,
                            const fluent_generator::Method *method,
                            std::map<std::string, std::string> *vars) {
  (*vars)["Method"] = method->name();
  (*vars)["Request"] = method->input_type_name();
  (*vars)["Response"] = method->output_type_name();

  if (method->NoStreaming()) {
    printer->Print(
        *vars,
        "MOCK_METHOD3($Method$, ::grpc::Status(::grpc::ClientContext* context, "
        "const $Request$& request, $Response$* response));\n");
    printer->Print(*vars,
                   "MOCK_METHOD3(Async$Method$Raw, "
                   "::grpc::ClientAsyncResponseReaderInterface< $Response$>*"
                   "(::grpc::ClientContext* context, const $Request$& request, "
                   "::grpc::CompletionQueue* cq));\n");
  } else if (ClientOnlyStreaming(method)) {
    printer->Print(
        *vars,
        "MOCK_METHOD2($Method$Raw, "
        "::grpc::ClientWriterInterface< $Request$>*"
        "(::grpc::ClientContext* context, $Response$* response));\n");
    printer->Print(*vars,
                   "MOCK_METHOD4(Async$Method$Raw, "
                   "::grpc::ClientAsyncWriterInterface< $Request$>*"
                   "(::grpc::ClientContext* context, $Response$* response, "
                   "::grpc::CompletionQueue* cq, void* tag));\n");
  } else if (ServerOnlyStreaming(method)) {
    printer->Print(
        *vars,
        "MOCK_METHOD2($Method$Raw, "
        "::grpc::ClientReaderInterface< $Response$>*"
        "(::grpc::ClientContext* context, const $Request$& request));\n");
    printer->Print(*vars,
                   "MOCK_METHOD4(Async$Method$Raw, "
                   "::grpc::ClientAsyncReaderInterface< $Response$>*"
                   "(::grpc::ClientContext* context, const $Request$& request, "
                   "::grpc::CompletionQueue* cq, void* tag));\n");
  } else if (method->BidiStreaming()) {
    printer->Print(
        *vars,
        "MOCK_METHOD1($Method$Raw, "
        "::grpc::ClientReaderWriterInterface< $Request$, $Response$>*"
        "(::grpc::ClientContext* context));\n");
    printer->Print(
        *vars,
        "MOCK_METHOD3(Async$Method$Raw, "
        "::grpc::ClientAsyncReaderWriterInterface<$Request$, $Response$>*"
        "(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, "
        "void* tag));\n");
  }
}

void PrintMockService(fluent_generator::Printer *printer,
                      const fluent_generator::Service *service,
                      std::map<std::string, std::string> *vars) {
  (*vars)["Service"] = service->name();

  printer->Print(*vars,
                 "class Mock$Service$Stub : public $Service$::StubInterface {\n"
                 " public:\n");
  printer->Indent();
  for (int i = 0; i < service->method_count(); ++i) {
    PrintMockClientMethods(printer, service->method(i).get(), vars);
  }
  printer->Outdent();
  printer->Print("};\n");
}

std::string GetMockServices(ProtoBufFile *file,
                            const Parameters &params) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    auto printer = file->CreatePrinter(&output);
    std::map<std::string, std::string> vars;
    // Package string is empty or ends with a dot. It is used to fully qualify
    // method names.
    vars["Package"] = file->package();
    if (!file->package().empty()) {
      vars["Package"].append(".");
    }

    if (!params.services_namespace.empty()) {
      vars["services_namespace"] = params.services_namespace;
      printer->Print(vars, "\nnamespace $services_namespace$ {\n\n");
    }

    for (int i = 0; i < file->service_count(); i++) {
      PrintMockService(printer.get(), file->service(i).get(), &vars);
      printer->Print("\n");
    }

    if (!params.services_namespace.empty()) {
      printer->Print(vars, "} // namespace $services_namespace$\n\n");
    }
  }
  return output;
}

std::string GetMockEpilogue(ProtoBufFile *file,
                            const Parameters & /*params*/) {
  std::string temp;

  if (!file->package().empty()) {
    std::vector<std::string> parts = file->package_parts();

    for (auto part = parts.begin(); part != parts.end(); part++) {
      temp.append("} // namespace ");
      temp.append(*part);
      temp.append("\n");
    }
    temp.append("\n");
  }

  return temp;
}

#endif

}  // namespace fluent_cpp_generator
