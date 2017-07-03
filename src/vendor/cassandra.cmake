ExternalProject_Add(cassandra_project
    GIT_REPOSITORY "https://github.com/datastax/cpp-driver"
    GIT_TAG "2.7.0"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(cassandra_project SOURCE_DIR)
ExternalProject_Get_Property(cassandra_project BINARY_DIR)
SET(CASSANDRA_INCLUDE_DIRS ${SOURCE_DIR}/include)
SET(CASSANDRA_LINK_DIRS ${BINARY_DIR})
