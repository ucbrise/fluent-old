ExternalProject_Add(range-v3_project
    GIT_REPOSITORY "https://github.com/ericniebler/range-v3"
    GIT_TAG "master"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
SET(RANGE-V3_PROJECT range-v3_project)

INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/src/range-v3_project/include)
