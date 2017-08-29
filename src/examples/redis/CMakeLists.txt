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

# Redox example.
CREATE_REDIS_EXECUTABLE(redox_example)

# Client and server.
CREATE_REDIS_EXECUTABLE(client)
CREATE_REDIS_EXECUTABLE(server)

# Benchmarks.
CREATE_REDIS_EXECUTABLE(benchmark_client)
CREATE_REDIS_EXECUTABLE(benchmark_lineage_server)
CREATE_REDIS_EXECUTABLE(benchmark_redox)
CREATE_REDIS_EXECUTABLE(benchmark_server)
