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

#ifndef GRPC_INTERNAL_COMPILER_CPP_GENERATOR_H
#define GRPC_INTERNAL_COMPILER_CPP_GENERATOR_H

// cpp_generator.h/.cc do not directly depend on GRPC/ProtoBuf, such that they
// can be used to generate code for other serialization systems, such as
// FlatBuffers.

#include <memory>
#include <vector>

#include "shim_gen/grpc/config.h"
#include "shim_gen/grpc/protobuf_plugin.h"
#include "shim_gen/grpc/schema_interface.h"

namespace fluent_cpp_generator {

// Contains all the parameters that are parsed from the command line.
struct Parameters {
  // TODO(mwhittaker): Add paramters as appropriate.
};

std::string GetPrologue(ProtoBufFile *file, const Parameters &params);
std::string GetIncludes(ProtoBufFile *file, const Parameters &params);
std::string GetClientClass(ProtoBufFile *file, const Parameters &params);
std::string GetApiFunction(ProtoBufFile *file, const Parameters &params);
std::string GetFluentFunction(ProtoBufFile *file, const Parameters &params);
std::string GetEpilogue(ProtoBufFile *file, const Parameters &params);

#if 0
// Return the prologue of the generated header file.
std::string GetHeaderPrologue(fluent_generator::File *file,
                              const Parameters &params);

// Return the includes needed for generated header file.
std::string GetHeaderIncludes(fluent_generator::File *file,
                              const Parameters &params);

// Return the includes needed for generated source file.
std::string GetSourceIncludes(fluent_generator::File *file,
                              const Parameters &params);

// Return the epilogue of the generated header file.
std::string GetHeaderEpilogue(fluent_generator::File *file,
                              const Parameters &params);

// Return the prologue of the generated source file.
std::string GetSourcePrologue(fluent_generator::File *file,
                              const Parameters &params);

// Return the services for generated header file.
std::string GetHeaderServices(fluent_generator::File *file,
                              const Parameters &params);

// Return the services for generated source file.
std::string GetSourceServices(fluent_generator::File *file,
                              const Parameters &params);

// Return the epilogue of the generated source file.
std::string GetSourceEpilogue(fluent_generator::File *file,
                              const Parameters &params);

// Return the prologue of the generated mock file.
std::string GetMockPrologue(fluent_generator::File *file,
                            const Parameters &params);

// Return the includes needed for generated mock file.
std::string GetMockIncludes(fluent_generator::File *file,
                            const Parameters &params);

// Return the services for generated mock file.
std::string GetMockServices(fluent_generator::File *file,
                            const Parameters &params);

// Return the epilogue of generated mock file.
std::string GetMockEpilogue(fluent_generator::File *file,
                            const Parameters &params);

// Return the prologue of the generated mock file.
std::string GetMockPrologue(fluent_generator::File *file,
                            const Parameters &params);

// Return the includes needed for generated mock file.
std::string GetMockIncludes(fluent_generator::File *file,
                            const Parameters &params);

// Return the services for generated mock file.
std::string GetMockServices(fluent_generator::File *file,
                            const Parameters &params);

// Return the epilogue of generated mock file.
std::string GetMockEpilogue(fluent_generator::File *file,
                            const Parameters &params);
#endif

}  // namespace fluent_cpp_generator

#endif  // GRPC_INTERNAL_COMPILER_CPP_GENERATOR_H
