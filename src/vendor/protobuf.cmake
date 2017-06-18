# https://goo.gl/iK7S1H
# https://goo.gl/K5QMBZ
ExternalProject_Add(protobuf_project
    GIT_REPOSITORY "https://github.com/google/protobuf"
    GIT_TAG "v3.3.0"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ./autogen.sh && ./configure
    BUILD_COMMAND make
    INSTALL_COMMAND ""
)

SET(PROTOBUF_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}/src/protobuf_project/src)
SET(PROTOBUF_LINK_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}/src/protobuf_project/src/.libs)

SET(Protobuf_SRC_ROOT_FOLDER
    ${CMAKE_CURRENT_BINARY_DIR}/src/protobuf_project)
SET(Protobuf_PROTOC_EXECUTABLE
    ${CMAKE_CURRENT_BINARY_DIR}/src/protobuf_project/src/protoc)
INCLUDE(FindProtobuf)
FIND_PACKAGE(Protobuf)
