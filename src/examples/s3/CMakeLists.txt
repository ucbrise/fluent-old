CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

MACRO(CREATE_S3_EXECUTABLE NAME)
    ADD_EXECUTABLE(examples_s3_${NAME} "${ARGN}" ${NAME})
    TARGET_LINK_LIBRARIES(examples_s3_${NAME}
        aws-cpp-sdk-core
        aws-cpp-sdk-s3
        common
        fluent
        fmt
        googlelog)
    ADD_DEPENDENCIES(examples_s3_${NAME}
        aws_project
        common
        fluent
        fmt
        googlelog)
ENDMACRO(CREATE_S3_EXECUTABLE)

CREATE_S3_EXECUTABLE(benchmark)
CREATE_S3_EXECUTABLE(client)
CREATE_S3_EXECUTABLE(example)
CREATE_S3_EXECUTABLE(server)
CREATE_S3_EXECUTABLE(server_benchmark)
CREATE_S3_EXECUTABLE(server_benchmark_lineage)
CREATE_S3_EXECUTABLE(client_benchmark)
