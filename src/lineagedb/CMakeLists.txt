CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_LIBRARY(lineagedb connection_config.cc)

TARGET_LINK_LIBRARIES(lineagedb
    ${FMT_LINK_DIRS}/libfmt.a
    ${LIBPQXX_LINK_DIRS}/libpqxx.a
    common
    ${GLOG_LINK_DIRS}/libglog.a
    pq
)

MACRO(CREATE_LINEAGEDB_TEST NAME)
    CREATE_NAMED_TEST(lineagedb_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(lineagedb_${NAME} lineagedb testing)
ENDMACRO(CREATE_LINEAGEDB_TEST)

CREATE_LINEAGEDB_TEST(mock_client_test)
CREATE_LINEAGEDB_TEST(mock_pqxx_client_test)
CREATE_LINEAGEDB_TEST(mock_to_sql_test)
CREATE_LINEAGEDB_TEST(to_sql_test)
