ExternalProject_Add(libpqxx_project
    GIT_REPOSITORY "https://github.com/jtv/libpqxx"
    GIT_TAG "5.0.1"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ./configure --disable-documentation --enable-shared=yes
    BUILD_COMMAND make
    INSTALL_COMMAND ""
)

INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/src/libpqxx_project/include)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/src/libpqxx_project/src/.libs)
