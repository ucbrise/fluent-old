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
SET(GTEST_INCLUDE_DIRS ${SOURCE_DIR}/googletest/include)

ExternalProject_Get_Property(googletest_project SOURCE_DIR)
SET(GMOCK_INCLUDE_DIRS ${SOURCE_DIR}/googlemock/include)

ExternalProject_Get_Property(googletest_project BINARY_DIR)
SET(GTEST_LIBS_DIR ${BINARY_DIR}/googlemock/gtest)

ExternalProject_Get_Property(googletest_project BINARY_DIR)
SET(GMOCK_LIBS_DIR ${BINARY_DIR}/googlemock)

ADD_LIBRARY(googletest STATIC IMPORTED)
SET_PROPERTY(TARGET googletest
             PROPERTY IMPORTED_LOCATION ${GTEST_LIBS_DIR}/libgtest.a)
ADD_DEPENDENCIES(googletest googletest_project)

ADD_LIBRARY(googletest_main STATIC IMPORTED)
SET_PROPERTY(TARGET googletest_main
             PROPERTY IMPORTED_LOCATION ${GTEST_LIBS_DIR}/libgtest_main.a)
ADD_DEPENDENCIES(googletest_main googletest_project)

ADD_LIBRARY(googlemock STATIC IMPORTED)
SET_PROPERTY(TARGET googlemock
             PROPERTY IMPORTED_LOCATION ${GMOCK_LIBS_DIR}/libgmock.a)
ADD_DEPENDENCIES(googlemock googletest_project)

ADD_LIBRARY(googlemock_main STATIC IMPORTED)
SET_PROPERTY(TARGET googlemock_main
             PROPERTY IMPORTED_LOCATION ${GMOCK_LIBS_DIR}/libgmock_main.a)
ADD_DEPENDENCIES(googlemock_main googletest_project)
