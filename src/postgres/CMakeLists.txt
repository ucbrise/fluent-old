CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_LIBRARY(postgres
    connection_config.cc
    constants.cc
    pqxx_client.cc
)

TARGET_LINK_LIBRARIES(postgres
    ${FMT_LINK_DIRS}/libfmt.a
    ${LIBPQXX_LINK_DIRS}/libpqxx.a
    common
    glog
    pq
)

MACRO(CREATE_POSTGRES_TEST NAME)
    CREATE_NAMED_TEST(postgres_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(postgres_${NAME} postgres)
ENDMACRO(CREATE_POSTGRES_TEST)

CREATE_POSTGRES_TEST(sql_type_test)
