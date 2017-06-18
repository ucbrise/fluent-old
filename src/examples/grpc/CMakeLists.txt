CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HDR api.proto)
GRPC_GENERATE_CPP(GRPC_SRC GRPC_HDR api.proto)
ADD_LIBRARY(examples_grcp_proto ${PROTO_SRC} ${GRPC_SRC})
SET_TARGET_PROPERTIES(examples_grcp_proto PROPERTIES COMPILE_FLAGS "-w")

MACRO(CREATE_GRPC_EXECUTABLE NAME)
    ADD_EXECUTABLE(examples_grpc_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(examples_grpc_${NAME}
        examples_grcp_proto
        fluent
        googlelog
        grpc
        grpc++
        protobuf
        pthread)
    ADD_DEPENDENCIES(examples_grpc_${NAME}
        examples_grcp_proto
        fluent
        googlelog
        grpc_project
        protobuf_project)
ENDMACRO(CREATE_GRPC_EXECUTABLE)

CREATE_GRPC_EXECUTABLE(client)
CREATE_GRPC_EXECUTABLE(server)
