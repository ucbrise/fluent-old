CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_EXECUTABLE(examples_test_client client.cc)
ADD_EXECUTABLE(examples_test_server server.cc)

TARGET_LINK_LIBRARIES(examples_test_client fluent)
TARGET_LINK_LIBRARIES(examples_test_server fluent)
