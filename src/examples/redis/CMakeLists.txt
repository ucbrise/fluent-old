CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

MACRO(CREATE_REDIS_EXECUTABLE NAME)
    ADD_EXECUTABLE(examples_redis_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(examples_redis_${NAME}
        ev
        fluent
        glog
        hiredis
        redox)
    ADD_DEPENDENCIES(examples_redis_${NAME}
        ${GOOGLELOG_PROJECT}
        ${REDOX_PROJECT}
        fluent)
ENDMACRO(CREATE_REDIS_EXECUTABLE)

CREATE_REDIS_EXECUTABLE(client)
CREATE_REDIS_EXECUTABLE(client_benchmark)
CREATE_REDIS_EXECUTABLE(redox_example)
CREATE_REDIS_EXECUTABLE(server)
CREATE_REDIS_EXECUTABLE(server_benchmark)
CREATE_REDIS_EXECUTABLE(server_benchmark_lineage)
CREATE_REDIS_EXECUTABLE(speed_test_sync)
