ExternalProject_Add(fmt_project
    GIT_REPOSITORY "https://github.com/fmtlib/fmt"
    GIT_TAG "3.0.0"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(fmt_project SOURCE_DIR)
SET(FMT_INCLUDE_DIRS ${SOURCE_DIR})

ExternalProject_Get_Property(fmt_project BINARY_DIR)
SET(FMT_LINK_DIRS ${BINARY_DIR}/fmt)

ADD_LIBRARY(fmt STATIC IMPORTED)
SET_PROPERTY(TARGET fmt PROPERTY IMPORTED_LOCATION ${FMT_LINK_DIRS}/libfmt.a)
ADD_DEPENDENCIES(fmt fmt_project)
