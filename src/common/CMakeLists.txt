CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(COMMON_SOURCES
    error_code.cc
    file_util.cc
    rand_util.cc
    status.cc
    string_util.cc)
ADD_LIBRARY(common ${COMMON_SOURCES})
ADD_LIBRARY(common_object OBJECT ${COMMON_SOURCES})

TARGET_LINK_LIBRARIES(common
    fmt
    glog
    pthread)

SET(COMMON_DEPENDENCIES
    ${BOOST_PROJECT}
    ${CEREAL_PROJECT}
    ${FMT_PROJECT}
    ${GOOGLELOG_PROJECT})
ADD_DEPENDENCIES(common ${COMMON_DEPENDENCIES})
ADD_DEPENDENCIES(common_object ${COMMON_DEPENDENCIES})

MACRO(CREATE_COMMON_TEST NAME)
    CREATE_NAMED_TEST(common_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(common_${NAME} common)
    ADD_DEPENDENCIES(common_${NAME} common)
ENDMACRO(CREATE_COMMON_TEST)

CREATE_COMMON_TEST(cereal_pickler_test)
CREATE_COMMON_TEST(collection_util_test)
CREATE_COMMON_TEST(hash_util_test)
CREATE_COMMON_TEST(macros_test)
CREATE_COMMON_TEST(rand_util_test)
CREATE_COMMON_TEST(sizet_list_test)
CREATE_COMMON_TEST(static_assert_test)
CREATE_COMMON_TEST(string_util_test)
CREATE_COMMON_TEST(time_util_test)
CREATE_COMMON_TEST(tuple_util_test)
CREATE_COMMON_TEST(type_list_test)
CREATE_COMMON_TEST(type_traits_test)
