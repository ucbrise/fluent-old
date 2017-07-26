ExternalProject_Add(grpc_project
    GIT_REPOSITORY "https://github.com/grpc/grpc"
    GIT_TAG "v1.3.4"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND git submodule update --init
    BUILD_COMMAND make
    INSTALL_COMMAND ""
)

SET(GRPC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/grpc_project)
SET(PROTOBUF_DIR ${GRPC_DIR}/third_party/protobuf)

INCLUDE_DIRECTORIES(SYSTEM ${PROTOBUF_DIR}/src)
LINK_DIRECTORIES(${PROTOBUF_DIR}/src/.libs)
SET(Protobuf_PROTOC_EXECUTABLE ${PROTOBUF_DIR}/src/protoc)

INCLUDE_DIRECTORIES(SYSTEM ${GRPC_DIR}/include)
LINK_DIRECTORIES(${GRPC_DIR}/libs/opt)
SET(gRPC_CPP_PLUGIN_EXECUTABLE ${GRPC_DIR}/bins/opt/grpc_cpp_plugin)
