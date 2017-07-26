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

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/src/grpc_project/include")
LINK_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/src/grpc_project/libs/opt")
SET(gRPC_CPP_PLUGIN_EXECUTABLE
    "${CMAKE_CURRENT_BINARY_DIR}/src/grpc_project/bins/opt/grpc_cpp_plugin")
