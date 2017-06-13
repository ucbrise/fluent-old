CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(ZMQ_UTIL_SOURCES socket_cache.cc zmq_util.cc)

ADD_LIBRARY(zmq_util ${ZMQ_UTIL_SOURCES})
ADD_LIBRARY(zmq_util_object OBJECT ${ZMQ_UTIL_SOURCES})

TARGET_LINK_LIBRARIES(zmq_util
    ${GLOG_LINK_DIRS}/libglog.a
    pthread
    zmq)

ADD_DEPENDENCIES(zmq_util
    googlelog
    zeromq
    zeromqcpp)

ADD_DEPENDENCIES(zmq_util_object
    googlelog
    zeromq
    zeromqcpp)
