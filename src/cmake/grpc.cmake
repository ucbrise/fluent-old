CMAKE_MINIMUM_REQUIRED(VERSION 3.0)


# CREATE_NAMED_TEST(foo bar.cc) creates a test named `foo` from the file
# `bar.cc`. Refer to [1] and [2] for documentation on how ADD_TEST works.
#
# [1]: http://stackoverflow.com/a/21413672/3187068
# [2]: https://cmake.org/cmake/help/v3.0/command/add_test.html
MACRO(GRPC_GENERATE_CPP GRPC_SRC GRPC_HDR FILENAME)
    GET_FILENAME_COMPONENT(FILENAME_NO_EXTENSION ${FILENAME} NAME_WE)
    SET(${GRPC_SRC}
        "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_NO_EXTENSION}.grpc.pb.cc")
    SET(${GRPC_HDR}
        "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME_NO_EXTENSION}.grpc.pb.h")

    SET(FULL_FILENAME)
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
