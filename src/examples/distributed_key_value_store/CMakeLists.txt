CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HDR api.proto)
GRPC_GENERATE_CPP(GRPC_SRC GRPC_HDR api.proto)
GRPC_SHIM_GENERATE_CPP(GRPC_SHIM_HDR api.proto)
ADD_LIBRARY(examples_distributed_key_value_store_proto ${PROTO_SRC} ${GRPC_SRC})
SET_TARGET_PROPERTIES(examples_distributed_key_value_store_proto
                      PROPERTIES COMPILE_FLAGS "-w")

MACRO(CREATE_DISTRIBUTED_KEY_VALUE_STORE_EXECUTABLE NAME)
    ADD_EXECUTABLE(examples_distributed_key_value_store_${NAME}
                   "${ARGN}"
                   ${GRPC_SHIM_HDR}
                   ${NAME})
    TARGET_LINK_LIBRARIES(examples_distributed_key_value_store_${NAME}
        examples_distributed_key_value_store_proto
        fluent
        glog
        grpc
        grpc++
        protobuf
        pthread)
    ADD_DEPENDENCIES(examples_distributed_key_value_store_${NAME}
        examples_distributed_key_value_store_proto
        fluent
        grpc_project)
ENDMACRO(CREATE_DISTRIBUTED_KEY_VALUE_STORE_EXECUTABLE)

CREATE_DISTRIBUTED_KEY_VALUE_STORE_EXECUTABLE(command_line_client client)
CREATE_DISTRIBUTED_KEY_VALUE_STORE_EXECUTABLE(fluent_client)
CREATE_DISTRIBUTED_KEY_VALUE_STORE_EXECUTABLE(fluent_server client)
CREATE_DISTRIBUTED_KEY_VALUE_STORE_EXECUTABLE(server client)
