CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_LIBRARY(fluent INTERFACE)
TARGET_LINK_LIBRARIES(fluent INTERFACE
    ${FMT_LINK_DIRS}/libfmt.a
    collections
    ${GLOG_LINK_DIRS}/libglog.a
    lineagedb
    zmq_util)

MACRO(CREATE_FLUENT_TEST NAME)
    CREATE_NAMED_TEST(fluent_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(fluent_${NAME} fluent testing)
ENDMACRO(CREATE_FLUENT_TEST)

CREATE_FLUENT_TEST(fluent_builder_test)
CREATE_FLUENT_TEST(fluent_executor_test)
CREATE_FLUENT_TEST(rule_test)
CREATE_FLUENT_TEST(infix_test)
