CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# GRPC_GENERATE_CPP(GRPC_SRC GRPC_HDR foo.proto) runs the grpc compiler on
# foo.proto to generate a source file (whose name is stored in GRPC_SRC) and a
# header file (whose name is stored in GRPC_HDR).
MACRO(GRPC_GENERATE_CPP GRPC_SRC GRPC_HDR FILENAME)
    GET_FILENAME_COMPONENT(FILENAME_NO_EXTENSION ${FILENAME} NAME_WE)
    SET(${GRPC_SRC}
        "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_NO_EXTENSION}.grpc.pb.cc")
    SET(${GRPC_HDR}
        "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_NO_EXTENSION}.grpc.pb.h")

    add_custom_command(
        OUTPUT "${${GRPC_SRC}}" "${${GRPC_HDR}}"
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
             -I "${CMAKE_CURRENT_SOURCE_DIR}"
             --plugin=protoc-gen-grpc="${gRPC_CPP_PLUGIN_EXECUTABLE}"
             "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}"
        DEPENDS ${FILENAME}
    )
ENDMACRO(GRPC_GENERATE_CPP)
