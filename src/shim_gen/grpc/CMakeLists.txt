CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_EXECUTABLE(cpp_plugin cpp_generator.cc cpp_plugin.cc)
TARGET_LINK_LIBRARIES(cpp_plugin protobuf protoc)
ADD_DEPENDENCIES(cpp_plugin protobuf_project)
