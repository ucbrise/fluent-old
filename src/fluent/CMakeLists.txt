CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_LIBRARY(fluent
    $<TARGET_OBJECTS:collections_object>
    $<TARGET_OBJECTS:common_object>
    $<TARGET_OBJECTS:zmq_util_object>
    $<TARGET_OBJECTS:lineagedb_object>)

TARGET_LINK_LIBRARIES(fluent
    collections
    common
    fmt
    gflags
    glog
    lineagedb
    pthread
    zmq_util)

ADD_DEPENDENCIES(fluent
    collections
    common
    fmt
    lineagedb
    zmq_util)

MACRO(CREATE_FLUENT_TEST NAME)
    CREATE_NAMED_TEST(fluent_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(fluent_${NAME} fluent testing)
    ADD_DEPENDENCIES(fluent_${NAME} fluent testing)
ENDMACRO(CREATE_FLUENT_TEST)

CREATE_FLUENT_TEST(fluent_builder_test)
CREATE_FLUENT_TEST(fluent_executor_test)
CREATE_FLUENT_TEST(rule_test)
CREATE_FLUENT_TEST(infix_test)
