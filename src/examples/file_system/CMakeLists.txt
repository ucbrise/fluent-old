CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_LIBRARY(examples_file_system string_store.cc)

MACRO(CREATE_FILE_SYSTEM_EXECUTABLE NAME)
    ADD_EXECUTABLE(examples_file_system_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(examples_file_system_${NAME}
        examples_file_system
        fluent)
    ADD_DEPENDENCIES(examples_file_system_${NAME}
        examples_file_system
        fluent)
ENDMACRO(CREATE_FILE_SYSTEM_EXECUTABLE)

CREATE_FILE_SYSTEM_EXECUTABLE(client)
CREATE_FILE_SYSTEM_EXECUTABLE(fluent_client)
CREATE_FILE_SYSTEM_EXECUTABLE(fluent_server)
CREATE_FILE_SYSTEM_EXECUTABLE(server)

MACRO(CREATE_FILE_SYSTEM_TEST NAME)
    CREATE_NAMED_TEST(examples_file_system_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(examples_file_system_${NAME} 
    	examples_file_system 
    	gflags)
ENDMACRO(CREATE_FILE_SYSTEM_TEST)

CREATE_FILE_SYSTEM_TEST(string_store_test)
