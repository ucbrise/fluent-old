ExternalProject_Add(googlelog_project
    GIT_REPOSITORY "https://github.com/google/glog"
    GIT_TAG "v0.3.5"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    CMAKE_ARGS -DBUILD_SHARED_LIBS=true
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(googlelog_project SOURCE_DIR)
ExternalProject_Get_Property(googlelog_project BINARY_DIR)
INCLUDE_DIRECTORIES(SYSTEM ${SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(SYSTEM ${BINARY_DIR})
LINK_DIRECTORIES(${BINARY_DIR})
