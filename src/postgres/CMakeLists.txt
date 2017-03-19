CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_LIBRARY(postgres
    connection_config.cc
    constants.cc
    pqxx_client.cc
)

TARGET_LINK_LIBRARIES(postgres
    ${FMT_LINK_DIRS}/libfmt.a
    ${LIBPQXX_LINK_DIRS}/libpqxx.a
    glog
    pq
)
