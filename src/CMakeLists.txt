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

# TODO(mwhittaker): Change compiler flags based on Debug or Release.
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
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

SET(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} \
     ${CMAKE_CXX_FLAGS_COMMON}")
SET(CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS_RELEASE} \
     ${CMAKE_CXX_FLAGS_COMMON}")

INCLUDE(ExternalProject)
INCLUDE(vendor/boost.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/cassandra.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/cereal.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/fmt.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/googlebenchmark.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/googlelog.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/googletest.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/grpc.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/libpqxx.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/protobuf.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/range-v3.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/redox.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/zeromq.cmake NO_POLICY_SCOPE)
INCLUDE(vendor/zeromqcpp.cmake NO_POLICY_SCOPE)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CASSANDRA_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CEREAL_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${GBENCH_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${GLOG_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${GMOCK_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${RANGEV3_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${ZEROMQCPP_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${ZEROMQ_INCLUDE_DIRS})
# We compile with the -Werror flag which means that warnings are turned into
# errors. The following includes produce warnings (turned errors) which prevent
# the code from building. By specifying them as a SYSTEM include, we ignore the
# warnings and allow the code to build.
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${FMT_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(SYSTEM ${GRPC_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(SYSTEM ${LIBPQXX_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(SYSTEM ${PROTOBUF_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(SYSTEM ${REDOX_INCLUDE_DIRS})

LINK_DIRECTORIES(${CASSANDRA_LINK_DIRS})
LINK_DIRECTORIES(${FMT_LINK_DIRS})
LINK_DIRECTORIES(${GLOG_LINK_DIRS})
LINK_DIRECTORIES(${GRPC_LINK_DIRS})
LINK_DIRECTORIES(${LIBPQXX_LINK_DIRS})
LINK_DIRECTORIES(${PROTOBUF_LINK_DIRS})
LINK_DIRECTORIES(${REDOX_LINK_DIRS})
LINK_DIRECTORIES(${ZEROMQ_LINK_DIRS})

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
