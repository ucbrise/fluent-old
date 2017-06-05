CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

include(ExternalProject)

ExternalProject_Add(fmt
    GIT_REPOSITORY "https://github.com/fmtlib/fmt"
    GIT_TAG "3.0.0"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(fmt SOURCE_DIR)
SET(FMT_INCLUDE_DIRS ${SOURCE_DIR} PARENT_SCOPE)

ExternalProject_Get_Property(fmt BINARY_DIR)
SET(FMT_LINK_DIRS ${BINARY_DIR}/fmt PARENT_SCOPE)
