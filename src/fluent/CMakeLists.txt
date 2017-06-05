CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_LIBRARY(fluent
    collection_util.cc
    socket_cache.cc
)
TARGET_LINK_LIBRARIES(fluent glog zmq_util lineagedb ${FMT_LINK_DIRS}/libfmt.a)

MACRO(CREATE_FLUENT_TEST NAME)
    CREATE_NAMED_TEST(fluent_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(fluent_${NAME} fluent testing)
ENDMACRO(CREATE_FLUENT_TEST)

CREATE_FLUENT_TEST(channel_test)
CREATE_FLUENT_TEST(collection_util_test)
CREATE_FLUENT_TEST(fluent_builder_test)
CREATE_FLUENT_TEST(fluent_executor_test)
CREATE_FLUENT_TEST(rule_test)
CREATE_FLUENT_TEST(scratch_test)
CREATE_FLUENT_TEST(socket_cache_test)
CREATE_FLUENT_TEST(stdout_test)
CREATE_FLUENT_TEST(table_test)
