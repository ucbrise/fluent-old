CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(COLLECTIONS_SOURCES collection_util.cc)

ADD_LIBRARY(collections ${COLLECTIONS_SOURCES})
ADD_LIBRARY(collections_object OBJECT ${COLLECTIONS_SOURCES})

TARGET_LINK_LIBRARIES(collections
    ${GLOG_LINK_DIRS}/libglog.a
    pthread
    zmq
    zmq_util)

ADD_DEPENDENCIES(collections
    googlelog
    zmq_util)

ADD_DEPENDENCIES(collections_object
    googlelog
    zmq_util)

MACRO(CREATE_COLLECTIONS_TEST NAME)
    CREATE_NAMED_TEST(collections_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(collections_${NAME} collections)
    ADD_DEPENDENCIES(collections_${NAME} collections)
ENDMACRO(CREATE_COLLECTIONS_TEST)

CREATE_COLLECTIONS_TEST(channel_test)
CREATE_COLLECTIONS_TEST(periodic_test)
CREATE_COLLECTIONS_TEST(scratch_test)
CREATE_COLLECTIONS_TEST(stdin_test)
CREATE_COLLECTIONS_TEST(stdout_test)
CREATE_COLLECTIONS_TEST(table_test)
CREATE_COLLECTIONS_TEST(collection_util_test)
