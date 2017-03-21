CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_EXECUTABLE(examples_indy_client client.cc)
ADD_EXECUTABLE(examples_indy_server server.cc)

TARGET_LINK_LIBRARIES(examples_indy_client fluent)
TARGET_LINK_LIBRARIES(examples_indy_server fluent)