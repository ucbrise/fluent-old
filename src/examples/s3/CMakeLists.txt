CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

MACRO(CREATE_S3_EXECUTABLE NAME)
    ADD_EXECUTABLE(examples_s3_${NAME} "${ARGN}" ${NAME})
    TARGET_LINK_LIBRARIES(examples_s3_${NAME}
        aws-cpp-sdk-core
        aws-cpp-sdk-s3
        fluent
        fmt
        googlelog)
    ADD_DEPENDENCIES(examples_s3_${NAME}
        aws_project
        fluent
        fmt
        googlelog)
ENDMACRO(CREATE_S3_EXECUTABLE)

CREATE_S3_EXECUTABLE(example)
CREATE_S3_EXECUTABLE(server)
CREATE_S3_EXECUTABLE(client)
