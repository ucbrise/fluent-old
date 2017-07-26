ExternalProject_Add(cassandra_project
    GIT_REPOSITORY "https://github.com/datastax/cpp-driver"
    GIT_TAG "2.7.0"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(cassandra_project SOURCE_DIR)
ExternalProject_Get_Property(cassandra_project BINARY_DIR)
INCLUDE_DIRECTORIES(SYSTEM ${SOURCE_DIR}/include)
LINK_DIRECTORIES(${BINARY_DIR})
