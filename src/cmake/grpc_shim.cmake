CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# GRPC_SHIM_GENERATE_CPP(GRPC_SHIM_HDR foo.proto) runs the grpc shim generator
# on foo.proto to generate a header file (whose name is stored in
# GRPC_SHIM_HDR).
MACRO(GRPC_SHIM_GENERATE_CPP GRPC_SHIM_HDR FILENAME)
    GET_FILENAME_COMPONENT(FILENAME_NO_EXTENSION ${FILENAME} NAME_WE)
    SET(${GRPC_SHIM_HDR}
        "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_NO_EXTENSION}.h")

    SET(FLUENT_PLUGIN "${CMAKE_BINARY_DIR}/bin/shim_gen_grpc_fluent_plugin")

    set(CMAKE_VERBOSE_MAKEFILE on)
    add_custom_command(
        OUTPUT "${${GRPC_SHIM_HDR}}"
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS --fluent_out "${CMAKE_CURRENT_BINARY_DIR}"
             -I "${CMAKE_CURRENT_SOURCE_DIR}"
             --plugin=protoc-gen-fluent="${FLUENT_PLUGIN}"
             "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}"
        DEPENDS ${FILENAME} shim_gen_grpc_fluent_plugin
    )
    set(CMAKE_VERBOSE_MAKEFILE off)
ENDMACRO(GRPC_SHIM_GENERATE_CPP)
