CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(Fluent)
SET(FLUENT_VERSION_MAJOR 0)
SET(FLUENT_VERSION_MINOR 1)
SET(FLUENT_VERSION_PATCH 0)

INCLUDE(cmake/benchmarking.cmake NO_POLICY_SCOPE)
INCLUDE(cmake/grpc.cmake NO_POLICY_SCOPE)
INCLUDE(cmake/grpc_shim.cmake NO_POLICY_SCOPE)
INCLUDE(cmake/protobuf.cmake NO_POLICY_SCOPE)
INCLUDE(cmake/testing.cmake NO_POLICY_SCOPE)

# Imagine the following file organization:
#
#   .
#   ├── examples
#   │   └── black_boxes
#   │       ├── CMakeLists.txt
#   │       └── key_value_client.cc
#   └── ra
#       ├── map.h
#       └── map_test.cc
#
# When we build key_value_client.cc, it produces an executable called
# examples_black_boxes_key_value_client. Similarly, when we build map_test.cc,
# it produces an executable called ra_map_test. Because we prefix every
# executable with its full path, we put all the executables into the same
# build/bin directory. This allows to run executables like this:
#
#   ./build/bin/examples_black_boxes_key_value_client
#
# instead of like this:
#
#   ./build/examples/black_boxes/examples_black_boxes_key_value_client
#
# See [1] for information on setting executable paths in CMake.
#
# [1]: http://stackoverflow.com/a/6595001/3187068
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

SET(CMAKE_CXX_FLAGS_COMMON
        "-std=c++14 \
         -Wall \
         -Wextra \
         -Werror \
         -pedantic")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Clang provides four sanitizers. Add them with the following flags:
    #   -fsanitize=address
    #   -fsanitize=thread
    #   -fsanitize=memory
    #   -fsanitize=undefined
    # Note that some sanitizers are mutually exclusive. For example, clang will
    # reject using -fsanitize=address and -fsanitize=thread simultaneously
    # simultaneously.
    #
    # Current CMAKE bug prevents using sanitizers under Xcode
    # https://gitlab.kitware.com/cmake/cmake/issues/16010
    if(NOT CMAKE_GENERATOR STREQUAL Xcode)
        SET(CMAKE_CXX_FLAGS_COMMON
            "${CMAKE_CXX_FLAGS_COMMON} \
            -fsanitize=address \
            -fsanitize=undefined")
    endif()
endif()

SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

SET(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} \
     ${CMAKE_CXX_FLAGS_COMMON}")
SET(CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS_RELEASE} \
     ${CMAKE_CXX_FLAGS_COMMON}")

# CMake provides a FIND_PACKAGE function that can be used to find packages that
# are already installed on a user's computer. For example, the following code
# attempts to find the BZip2 package:
#
#     FIND_PACKAGE (BZip2)
#     IF (BZIP2_FOUND)
#         INCLUDE_DIRECTORIES(${BZIP_INCLUDE_DIRS})
#         TARGET_LINK_LIBRARIES (helloworld ${BZIP2_LIBRARIES})
#     ENDIF ()
#
# CMake cannot find arbitrary packages. It can only find the ones it is
# configured to find. Run `cmake --help-module-list` for a list of all the
# packages CMake can find by default. However, CMake has a plug in system which
# lets you tell CMake how to find other packages that is doesn't know how to
# find by default. You do so by writing special module files; see [1] for an in
# depth explanation. This line specifies the directory in which we place module
# files.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

INCLUDE(ExternalProject)
INCLUDE(vendor/boost.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/cereal.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/fmt.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/googlebenchmark.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/googlelog.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/googletest.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/grpc.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/libpqxx.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/range-v3.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/redox.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/zeromq.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/zeromqcpp.cmake NO_POLICY_SCOPE)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

ENABLE_TESTING()
ADD_SUBDIRECTORY(collections)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(fluent)
ADD_SUBDIRECTORY(lineagedb)
ADD_SUBDIRECTORY(ra)
ADD_SUBDIRECTORY(shim_gen)
ADD_SUBDIRECTORY(testing)
ADD_SUBDIRECTORY(zmq_util)
