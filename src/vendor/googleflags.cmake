# Resources:
#   - http://www.kaizou.org/2014/11/gtest-cmake/
#   - http://bit.ly/2cx70Pk
#   - https://github.com/snikulov/google-test-examples
ExternalProject_Add(googleflags_project
    GIT_REPOSITORY "https://github.com/gflags/gflags"
    GIT_TAG "v2.2.1"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(googleflags_project BINARY_DIR)
SET(GFLAGS_LIBS_DIR ${BINARY_DIR}/lib)

ADD_LIBRARY(googleflags STATIC IMPORTED)
SET_PROPERTY(TARGET googleflags
             PROPERTY IMPORTED_LOCATION ${GFLAGS_LIBS_DIR}/libgflags.a)
ADD_DEPENDENCIES(googleflags googleflags_project)
