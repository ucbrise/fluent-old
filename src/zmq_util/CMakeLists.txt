CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(ZMQ_UTIL_SOURCES socket_cache.cc zmq_util.cc)
ADD_LIBRARY(zmq_util ${ZMQ_UTIL_SOURCES})
ADD_LIBRARY(zmq_util_object OBJECT ${ZMQ_UTIL_SOURCES})

TARGET_LINK_LIBRARIES(zmq_util
    glog
    pthread
    zmq)

SET(ZMQ_UTIL_DEPENDENCIES
	zeromq_project
    zeromqcpp_project)
ADD_DEPENDENCIES(zmq_util ${ZMQ_UTIL_DEPENDENCIES})
ADD_DEPENDENCIES(zmq_util_object ${ZMQ_UTIL_DEPENDENCIES})
