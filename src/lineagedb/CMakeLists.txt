CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(LINEAGEDB_SOURCES connection_config.cc)
ADD_LIBRARY(lineagedb ${LINEAGEDB_SOURCES})
ADD_LIBRARY(lineagedb_object OBJECT ${LINEAGEDB_SOURCES})

TARGET_LINK_LIBRARIES(lineagedb
    common
    fmt
    glog
    libpqxx
    pq
    pthread)

SET(LINEAGEDB_DEPENDENCIES
    common
    fmt
    libpqxx)
ADD_DEPENDENCIES(lineagedb ${LINEAGEDB_DEPENDENCIES})
ADD_DEPENDENCIES(lineagedb_object ${LINEAGEDB_DEPENDENCIES})

MACRO(CREATE_LINEAGEDB_TEST NAME)
    CREATE_NAMED_TEST(lineagedb_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(lineagedb_${NAME} lineagedb testing)
    ADD_DEPENDENCIES(lineagedb_${NAME} lineagedb testing)
ENDMACRO(CREATE_LINEAGEDB_TEST)

CREATE_LINEAGEDB_TEST(mock_client_test)
CREATE_LINEAGEDB_TEST(mock_pqxx_client_test)
CREATE_LINEAGEDB_TEST(mock_to_sql_test)
CREATE_LINEAGEDB_TEST(to_sql_test)
