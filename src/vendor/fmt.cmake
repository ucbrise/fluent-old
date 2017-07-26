ExternalProject_Add(fmt_project
    GIT_REPOSITORY "https://github.com/fmtlib/fmt"
    GIT_TAG "3.0.0"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    CMAKE_ARGS -DBUILD_SHARED_LIBS=true
    INSTALL_COMMAND ""
)
SET(FMT_PROJECT fmt_project)

ExternalProject_Get_Property(fmt_project SOURCE_DIR)
ExternalProject_Get_Property(fmt_project BINARY_DIR)
INCLUDE_DIRECTORIES(SYSTEM ${SOURCE_DIR})
LINK_DIRECTORIES(${BINARY_DIR}/fmt)
