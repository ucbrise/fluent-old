# Resources:
#   - http://www.kaizou.org/2014/11/gtest-cmake/
#   - http://bit.ly/2cx70Pk
#   - https://github.com/snikulov/google-test-examples
ExternalProject_Add(googlebenchmark_project
    GIT_REPOSITORY "https://github.com/google/benchmark"
    GIT_TAG "v1.1.0"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    CMAKE_ARGS -DBUILD_SHARED_LIBS=true
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(googlebenchmark_project SOURCE_DIR)
ExternalProject_Get_Property(googlebenchmark_project BINARY_DIR)
INCLUDE_DIRECTORIES(SYSTEM ${SOURCE_DIR}/include)
LINK_DIRECTORIES(${BINARY_DIR}/src)
