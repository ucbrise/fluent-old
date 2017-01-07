CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

include(ExternalProject)

ExternalProject_Add(range-v3
    GIT_REPOSITORY "https://github.com/ericniebler/range-v3"
    GIT_TAG "master"
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

 set(RANGEV3_INCLUDE_DIRS
     ${CMAKE_CURRENT_BINARY_DIR}/range-v3-prefix/src/range-v3/include
     PARENT_SCOPE)
