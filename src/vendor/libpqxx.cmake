ExternalProject_Add(libpqxx_project
    GIT_REPOSITORY "https://github.com/jtv/libpqxx"
    GIT_TAG "5.0.1"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ./configure --disable-documentation
    BUILD_COMMAND make
    INSTALL_COMMAND ""
)

SET(LIBPQXX_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}/src/libpqxx_project/include)
SET(LIBPQXX_LINK_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}/src/libpqxx_project/src/.libs)

ADD_LIBRARY(libpqxx STATIC IMPORTED)
SET_PROPERTY(TARGET libpqxx
             PROPERTY IMPORTED_LOCATION ${LIBPQXX_LINK_DIRS}/libpqxx.a)
ADD_DEPENDENCIES(libpqxx libpqxx_project)
