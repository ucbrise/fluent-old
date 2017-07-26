# Resources:
#   - https://github.com/rsakamoto/dexter/blob/13bac2f372bf0beeba673187c39d657978d71890/ext/glog/CMakeLists.txt
ExternalProject_Add(googlelog_project
    GIT_REPOSITORY "https://github.com/google/glog"
    GIT_TAG "v0.3.5"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(googlelog_project SOURCE_DIR)
ExternalProject_Get_Property(googlelog_project BINARY_DIR)
SET(GLOG_INCLUDE_DIRS ${SOURCE_DIR}/src ${BINARY_DIR})
SET(GLOG_LINK_DIRS ${BINARY_DIR})

ADD_LIBRARY(googlelog STATIC IMPORTED)
SET_PROPERTY(TARGET googlelog
             PROPERTY IMPORTED_LOCATION ${GLOG_LINK_DIRS}/libglog.a)
ADD_DEPENDENCIES(googlelog googlelog_project)
