#FIND_PACKAGE(Glog)

#if (GLOG_FOUND)
    #SET(GLOG_INCLUDE_DIRS ${GLOG_INCLUDE_DIRS})
    #SET(GLOG_LINK_DIRS ${GLOG_LIBRARIES})
#else ()
    # Resources:
    #   - https://github.com/rsakamoto/dexter/blob/13bac2f372bf0beeba673187c39d657978d71890/ext/glog/CMakeLists.txt
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
#endif ()
