CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_EXECUTABLE(examples_twitter_client client.cc)
ADD_EXECUTABLE(examples_twitter_server server.cc)

TARGET_LINK_LIBRARIES(examples_twitter_client fluent)
TARGET_LINK_LIBRARIES(examples_twitter_server fluent)
