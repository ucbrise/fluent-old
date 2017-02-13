CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(ToBeNamed)
SET(TOBENAMED_VERSION_MAJOR 0)
SET(TOBENAMED_VERSION_MINOR 1)
SET(TOBENAMED_VERSION_PATCH 0)

INCLUDE(cmake/benchmarking.cmake NO_POLICY_SCOPE)
INCLUDE(cmake/testing.cmake NO_POLICY_SCOPE)

# Clang provides four sanitizers. Add them with the following flags:
#   -fsanitize=address
#   -fsanitize=thread
#   -fsanitize=memory
#   -fsanitize=undefined
# Note that some sanitizers are mutually exclusive. For example, clang will
# reject using -fsanitize=address and -fsanitize=thread simultaneously
# simultaneously.
SET(CMAKE_CXX_FLAGS_COMMON
    "-std=c++14 \
     -O0 \
     -Wall \
     -Wextra \
     -Werror \
     -pedantic \
     -g")
SET(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} \
     ${CMAKE_CXX_FLAGS_COMMON}")
SET(CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS_RELEASE} \
     ${CMAKE_CXX_FLAGS_COMMON}")

ADD_SUBDIRECTORY(vendor/googlebenchmark)
ADD_SUBDIRECTORY(vendor/googlelog)
ADD_SUBDIRECTORY(vendor/googletest)
ADD_SUBDIRECTORY(vendor/range-v3)
ADD_SUBDIRECTORY(vendor/zeromq)
ADD_SUBDIRECTORY(vendor/zeromqcpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${GBENCH_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${GLOG_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${GMOCK_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${RANGEV3_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${ZEROMQCPP_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${ZEROMQ_INCLUDE_DIRS})

LINK_DIRECTORIES(${GLOG_LINK_DIRS})
LINK_DIRECTORIES(${ZEROMQ_LINK_DIRS})

ENABLE_TESTING()
ADD_SUBDIRECTORY(example)
#ADD_SUBDIRECTORY(fluent)
ADD_SUBDIRECTORY(ra)
ADD_SUBDIRECTORY(testing)
ADD_SUBDIRECTORY(zmq_util)
