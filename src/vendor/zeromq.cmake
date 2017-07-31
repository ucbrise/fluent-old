# Resources:
#  - http://zeromq.org/intro:get-the-software
#  - http://bit.ly/2dK0UBT
#
# Remember to have libtool, pkg-config, build-essential, autoconf, and automake
# installed.
ExternalProject_Add(zeromq_project
    URL "https://github.com/zeromq/zeromq4-1/releases/download/v4.1.5/zeromq-4.1.5.tar.gz"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ./configure
    BUILD_COMMAND make
    INSTALL_COMMAND ""
)

INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/src/zeromq_project/include)
LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/src/zeromq_project/.libs)
