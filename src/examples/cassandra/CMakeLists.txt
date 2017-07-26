CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

MACRO(CREATE_CASSANDRA_EXECUTABLE NAME)
    ADD_EXECUTABLE(examples_cassandra_${NAME} "${ARGN}" ${NAME})
    TARGET_LINK_LIBRARIES(examples_cassandra_${NAME}
        cassandra
        fluent)
    ADD_DEPENDENCIES(examples_cassandra_${NAME}
        ${CASSANDRA_PROJECT}
        fluent)
ENDMACRO(CREATE_CASSANDRA_EXECUTABLE)

CREATE_CASSANDRA_EXECUTABLE(benchmark_client_getter workloads)
CREATE_CASSANDRA_EXECUTABLE(benchmark_client_setter workloads)
CREATE_CASSANDRA_EXECUTABLE(client)
CREATE_CASSANDRA_EXECUTABLE(server)
