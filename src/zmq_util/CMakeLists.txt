CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_LIBRARY(zmq_util socket_cache.cc zmq_util.cc)
TARGET_LINK_LIBRARIES(zmq_util glog zmq)
