# Resources:
#   - http://www.kaizou.org/2014/11/gtest-cmake/
#   - http://bit.ly/2cx70Pk
#   - https://github.com/snikulov/google-test-examples
ExternalProject_Add(googlebenchmark_project
    GIT_REPOSITORY "https://github.com/google/benchmark"
    GIT_TAG "v1.1.0"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(googlebenchmark_project SOURCE_DIR)
SET(GBENCH_INCLUDE_DIRS ${SOURCE_DIR}/include)

ExternalProject_Get_Property(googlebenchmark_project BINARY_DIR)
SET(GBENCH_LIBS_DIR ${BINARY_DIR}/src)

ADD_LIBRARY(googlebenchmark STATIC IMPORTED)
SET_PROPERTY(TARGET googlebenchmark
             PROPERTY IMPORTED_LOCATION ${GBENCH_LIBS_DIR}/libbenchmark.a)
ADD_DEPENDENCIES(googlebenchmark googlebenchmark_project)
