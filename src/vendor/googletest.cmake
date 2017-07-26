# Resources:
#   - http://www.kaizou.org/2014/11/gtest-cmake/
#   - http://bit.ly/2cx70Pk
#   - https://github.com/snikulov/google-test-examples
ExternalProject_Add(googletest_project
    GIT_REPOSITORY "https://github.com/google/googletest"
    GIT_TAG "release-1.8.0"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(googletest_project SOURCE_DIR)
ExternalProject_Get_Property(googletest_project BINARY_DIR)
INCLUDE_DIRECTORIES(SYSTEM ${SOURCE_DIR}/googletest/include)
INCLUDE_DIRECTORIES(SYSTEM ${SOURCE_DIR}/googlemock/include)
LINK_DIRECTORIES(${BINARY_DIR}/googlemock/gtest)
LINK_DIRECTORIES(${BINARY_DIR}/googlemock)

ADD_LIBRARY(gtest STATIC IMPORTED)
SET_PROPERTY(TARGET gtest
    PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/googlemock/gtest/libgtest.a)
ADD_DEPENDENCIES(gtest googletest_project)

ADD_LIBRARY(gtest_main STATIC IMPORTED)
SET_PROPERTY(TARGET gtest_main
    PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/googlemock/gtest/libgtest_main.a)
ADD_DEPENDENCIES(gtest_main googletest_project)

ADD_LIBRARY(gmock STATIC IMPORTED)
SET_PROPERTY(TARGET gmock
    PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/googlemock/libgmock.a)
ADD_DEPENDENCIES(gmock googletest_project)

ADD_LIBRARY(gmock_main STATIC IMPORTED)
SET_PROPERTY(TARGET gmock_main
    PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/googlemock/libgmock_main.a)
ADD_DEPENDENCIES(gmock_main googletest_project)
