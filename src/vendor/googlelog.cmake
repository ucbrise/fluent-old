# Resources:
#   - https://github.com/rsakamoto/dexter/blob/13bac2f372bf0beeba673187c39d657978d71890/ext/glog/CMakeLists.txt
ExternalProject_Add(googlelog_project
    GIT_REPOSITORY "https://github.com/google/glog"
    GIT_TAG "master"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ./autogen.sh && ./configure --with-gflags --with-sysroot=${GFLAGS_LIBS_DIR}
    BUILD_COMMAND make
    INSTALL_COMMAND ""
)

SET(GLOG_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src/googlelog_project/src)
SET(GLOG_LINK_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src/googlelog_project/.libs)

ADD_LIBRARY(googlelog STATIC IMPORTED)
SET_PROPERTY(TARGET googlelog
             PROPERTY IMPORTED_LOCATION ${GLOG_LINK_DIRS}/libglog.a)
ADD_DEPENDENCIES(googlelog googlelog_project)
