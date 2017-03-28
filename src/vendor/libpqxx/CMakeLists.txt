CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

include(ExternalProject)

ExternalProject_Add(libpqxx
    GIT_REPOSITORY "https://github.com/jtv/libpqxx"
    GIT_TAG "5.0.1"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ./configure --disable-documentation
    BUILD_COMMAND make
    INSTALL_COMMAND ""
)

set(LIBPQXX_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src/libpqxx/include PARENT_SCOPE)
set(LIBPQXX_LINK_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src/libpqxx/src/.libs PARENT_SCOPE)
